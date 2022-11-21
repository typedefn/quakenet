/*
 * ExploreGoal.cpp 
 *
 *  Created on: Oct 1, 2021
 *      Author: dervish
 */

#include "ExploreGoal.hpp"
#include "Bot.hpp"

ExploreGoal::ExploreGoal(Bot *owner) {
  this->owner = owner;
  rot = glm::mat4(1);
  timer = owner->getTime();
  visible = false;
  myYaw = 0.0f;
  scanRange = 150;
  hitSensor = 0;
}

ExploreGoal::~ExploreGoal() {
}

void ExploreGoal::update() {
  PlayerInfo *me = owner->getMe();
  std::vector<glm::vec3> positions;
	int pitch = owner->getAngleX();

	glm::quat pitchQ = glm::angleAxis(glm::radians((float)pitch), glm::vec3(1, 0, 0));
	glm::quat rollQ = glm::angleAxis(glm::radians((float)0), glm::vec3(0, 0, 1));
  int yaw[3] = {0};
  int headingYaw[3] = {0};
  glm::vec3 headings[3];
  glm::vec3 rights[3];

  { // left sensor
    yaw[0] = owner->getAngleY() - 85;
		glm::quat yawQ = glm::angleAxis(glm::radians((float)yaw[0]), glm::vec3(0, 1, 0));
		glm::quat orientation = pitchQ * yawQ * rollQ;
		orientation = glm::normalize(orientation);
		glm::mat4 rotate = glm::mat4_cast(orientation);

		headings[0] = (glm::vec3(glm::inverse(rotate) * glm::vec4(0, 0, 1, 1)));
    rights[0] = glm::vec3(glm::inverse(rotate) * glm::vec4(1, 0, 0, 1));
		glm::vec3 targetPosition = me->position + scanRange * glm::normalize(headings[0]);
		positions.push_back(targetPosition);
	  headingYaw[0] = atan2(-headings[0].z, headings[0].x) * (180.0 / PI);
  }

  { // center sensor
    yaw[1] = owner->getAngleY();
		glm::quat yawQ = glm::angleAxis(glm::radians((float)yaw[1]), glm::vec3(0, 1, 0));
		glm::quat orientation = pitchQ * yawQ * rollQ;
		orientation = glm::normalize(orientation);
		glm::mat4 rotate = glm::mat4_cast(orientation);

		headings[1] = (glm::vec3(glm::inverse(rotate) * glm::vec4(0, 0, 1, 1)));
    rights[1] = glm::vec3(glm::inverse(rotate) * glm::vec4(1, 0, 0, 1));
		glm::vec3 targetPosition = me->position + scanRange * glm::normalize(headings[1]);
		positions.push_back(targetPosition);
	  headingYaw[1] = atan2(-headings[1].z, headings[1].x) * (180.0 / PI);
  }

  { // right sensor
    yaw[2] = owner->getAngleY() + 85;
		glm::quat yawQ = glm::angleAxis(glm::radians((float)yaw[2]), glm::vec3(0, 1, 0));
		glm::quat orientation = pitchQ * yawQ * rollQ;
		orientation = glm::normalize(orientation);
		glm::mat4 rotate = glm::mat4_cast(orientation);

		headings[2] = (glm::vec3(glm::inverse(rotate) * glm::vec4(0, 0, 1, 1)));
    rights[2] = glm::vec3(glm::inverse(rotate) * glm::vec4(1, 0, 0, 1));
		glm::vec3 targetPosition = me->position + scanRange * glm::normalize(headings[2]);
	  positions.push_back(targetPosition);
	  headingYaw[2] = atan2(-headings[2].z, headings[2].x) * (180.0 / PI);
  }


	double prevTimer = timer;
	float dist = 0;
  if (owner->getTime() - prevTimer > 0.1) {
		for(int i = 0; i < positions.size(); ++i) {
			glm::vec3 p = positions.at(i);
			visible = pathIsVisible(p, &dist);
      if (!visible) {
        hitSensor = i;
        break;
      }
		}
    timer = owner->getTime();
  }

	if (visible) {
		LOG << "NOT HIT WALL";
		owner->moveForward(250);
	} else {
		LOG << "HIT WALL ";
    glm::vec3 heading = rights[hitSensor]; 
	  float angleY = 90 + atan2(-heading.z, heading.x) * (180.0 / PI);
		owner->rotateY((int(angleY)) % 360);
	}
/*
  if (scanRange >= 150) {
    movement = true;
  }

  if (movement) {
    int dist = glm::distance(gotoPosition, me->position);
    glm::vec3 dir = gotoPosition - me->position;
    float yaw = 90 + (atan2(-dir.x, dir.z) * (180.0 / PI));

    if (dist >= 100) {
      owner->moveForward(300);
    } else {
      movement = false;
      LOG << "reached";
      owner->moveForward(0);
    }

    owner->rotateY(yaw);
  }
*/
}

double ExploreGoal::calculateDesirability() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();

  double desire = 0.1;
  double tweak = 2;

  if (!targetingSystem->isTargetPresent()) {
    desire = tweak;
  }

  return desire;
}

bool ExploreGoal::pathIsVisible(glm::vec3 targetPosition, float *dist) {
  PlayerInfo * me = owner->getMe();
  glm::vec3 myPosition = me->position;
  float d = 100;
  float c = 50;
  glm::vec3 tmin = glm::vec3(-c, -d, -c);
  glm::vec3 tmax = glm::vec3(c, d, c);

  Box targetBox(tmin + targetPosition, tmax + targetPosition);

  float distanceToTarget = glm::distance(myPosition, targetPosition);

  glm::vec3 direction = targetPosition - myPosition;

  Ray ray(myPosition, glm::normalize(direction));

  float t2 = 0.0;
  bool targetIntersected = targetBox.intersectV2(ray, &t2);
  float offset = 999999;
  float smallestT1 = offset;
  Model bspModel = owner->getBspModel();

  for (auto & surface : bspModel.surfaces) {
    std::string textureName = surface.texinfo.texture.name;
 
    if (textureName == "trigger") {
      continue;
    }

    glm::vec3 surfMin(offset);
    glm::vec3 surfMax(-offset);

    Polygon * p = &surface.polys;

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
  }
  *dist = t2;
  return t2 < smallestT1;
}
