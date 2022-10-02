/*
 * TargetingSystem.cpp
 *
 *  Created on: Sep 25, 2021
 *      Author: mastakillah
 */

#include "TargetingSystem.hpp"
#include "Bot.hpp"

TargetingSystem::TargetingSystem(Bot *owner) {
	// TODO Auto-generated constructor stub
	this->owner = owner;
	currentTarget = -1;
}

TargetingSystem::~TargetingSystem() {
	// TODO Auto-generated destructor stub
}

void TargetingSystem::update() {
	double closestDistanceSoFar = 999999999.0;

	std::list<int> entities =
			owner->getBotMemory()->getListOfRecentlySensedEntities();
  PlayerInfo *m = owner->getMe();
	for (const auto id : entities) {
		PlayerInfo *p = owner->getPlayerBySlot(id);
                 
		if (m->slot == id) {
			continue;
		}

		if (!p->active) {
			continue;
		}

		float dist = glm::distance(owner->getBotMemory()->getLastSensedPosition(id), m->position);

		if (dist < closestDistanceSoFar && dist <= owner->getBotConfig().targetDistance) {
			closestDistanceSoFar = dist;
                        currentTarget = id;
		}
	}

                  if (!targetIsVisible(currentTarget)) {
	            clearTarget();	
                  }


}

bool TargetingSystem::isTargetPresent() const {
	return currentTarget != -1 && currentTarget != owner->getMe()->slot;
}

bool TargetingSystem::isTargetWithinFov() const {
	return owner->getBotMemory()->isWithinFov(currentTarget);
}

glm::vec3 TargetingSystem::getLastRecordedPosition() const {
	return owner->getBotMemory()->getLastSensedPosition(currentTarget);
}

double TargetingSystem::getTimeTargetHasBeenVisible() const {
	return owner->getBotMemory()->getTimeEntityHasBeenVisible(currentTarget);
}

double TargetingSystem::getTimeTargetHasBeenOutOfView() const {
	return owner->getBotMemory()->getTimeEntityHasBeenOutOfFov(currentTarget);
}

int TargetingSystem::getTarget() const {
	return currentTarget;
}

void TargetingSystem::clearTarget() {
	currentTarget = -1;
}

bool TargetingSystem::targetIsVisible(int id) {

  if (!isTargetPresent()) {
    return false;
  }

  PlayerInfo *target = owner->getPlayerBySlot(id);
  PlayerInfo *me = owner->getMe();

  glm::vec3 targetPosition = target->position;
  glm::vec3 myPosition = me->position;

  glm::vec3 tmin = glm::vec3(-50, -100, -50);
  glm::vec3 tmax = glm::vec3(50, 100, 50);

  Box targetBox(tmin + targetPosition, tmax + targetPosition);

  float distanceToTarget = glm::distance(myPosition, targetPosition);

  glm::vec3 direction = targetPosition - myPosition;

  Ray ray(myPosition, glm::normalize(direction));

  float t2 = 0.0;
  bool targetIntersected = targetBox.intersectV2(ray, &t2);
  float offset = 99999;
  float smallestT1 = 99999;

  for (auto & surface : bspModel.surfaces) {
    std::string textureName = surface.texinfo.texture.name;

    glm::vec3 surfMin(offset);
    glm::vec3 surfMax(-offset);

    Polygon *p = &surface.polys;

    for (size_t j = 0; j < p->verts.size(); ++j) {
      glm::vec3 p0 = p->verts.at(j).position;
      MathUtil::instance()->findMin(p0, surfMin);
      MathUtil::instance()->findMax(p0, surfMax);
    }

    Box box(surfMin, surfMax);
    box.setCenter((box.getMin() + box.getMax()) / 2.0f);

    glm::vec3 po = box.getCenter();

    float dist = glm::distance(myPosition, po);
    float t = 0.0;

    bool rayIntersected = box.intersectV2(ray, &t);

    if (targetIntersected && rayIntersected && smallestT1 > t) {
      smallestT1 = t;
    } 
  } // end of surfaces

  return t2 < smallestT1;
}
