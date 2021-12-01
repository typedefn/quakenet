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
	currentTarget = -1;

	list<int> entities =
			owner->getBotMemory()->getListOfRecentlySensedEntities();

	for (const auto id : entities) {
		PlayerInfo *p = owner->getPlayerById(id);
		PlayerInfo *m = owner->getMe();

		LOG << " id = " << id;

		if (p == m) {
			continue;
		}

		LOG << " p->active = " << p->active;

		if (!p->active) {
			continue;
		}

		glm::vec3 targetPosition = glm::vec3(p->coords[0], p->coords[2], p->coords[1]);
		glm::vec3 botPosition = glm::vec3(m->coords[0], m->coords[2], m->coords[1]);

		float dist = glm::distance(targetPosition, botPosition);

		LOG << " dist = " << dist;

		if (dist < closestDistanceSoFar) {
			closestDistanceSoFar = dist;
			currentTarget = id;
		}
	}
}

bool TargetingSystem::isTargetPresent() const {
	return currentTarget != -1;
}

bool TargetingSystem::isTargetWithinFov() const {
	return owner->getBotMemory()->isWithinFov(currentTarget);
}

vec3 TargetingSystem::getLastRecordedPosition() const {
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
