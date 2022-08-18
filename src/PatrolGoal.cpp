/*
 * PatrolGoal.cpp
 *
 *  Created on: Sep 30, 2021
 *      Author: dervish
 */

#include "PatrolGoal.hpp"
#include "Bot.hpp"

PatrolGoal::PatrolGoal(Bot *owner) {
  this->owner = owner;
  currentTime = 0;
  totalTime = 0;
  dist = 0;
}

PatrolGoal::~PatrolGoal() {
}

void PatrolGoal::update() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory *memory = owner->getBotMemory();
  PlayerInfo *me = owner->getMe();
  std::vector<glm::vec3> waypoints = owner->getWaypoints()["start"];

  static int wi = 0;
  const float maxDistance = 50.0;
  glm::vec3 targetPosition(waypoints.at(wi).x, me->position.y, waypoints.at(wi).z);

  dist = glm::distance(targetPosition, me->position);
  glm::vec3 dir = normalize(targetPosition - me->position);
  float yaw = 90 + atan2(-dir.x, dir.z) * (180.0 / PI);

  if (dist > maxDistance) {
    owner->moveForward(glm::min(248, dist+50));
//    LOG << " distance " << dist;
  }

  owner->rotateY(yaw);
  owner->rotateX(0);
}

double PatrolGoal::calculateDesirability() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory *memory = owner->getBotMemory();
  double desire = 0.001;
  double tweak = 1.0;
  if (!targetingSystem->isTargetPresent()) {
    desire = tweak * owner->getHealth()/100.0;
  }
  return desire;
}
