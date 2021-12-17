/*
 * PatrolGoal.cpp
 *
 *  Created on: Sep 30, 2021
 *      Author: dervish
 */

#include "PatrolGoal.hpp"
#include "Bot.hpp"

PatrolGoal::PatrolGoal(Bot *owner) {
  // TODO Auto-generated constructor stub
  this->owner = owner;
  currentTime = 0;
  totalTime = 0;
  previousDist = 0;
  dist = 0;
}

PatrolGoal::~PatrolGoal() {
  // TODO Auto-generated destructor stub
}

void PatrolGoal::update() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory *memory = owner->getBotMemory();
  PlayerInfo *me = owner->getMe();
  Command *cmd = owner->getCommand();

  double previousTime = currentTime;
  currentTime = owner->getTime();

  totalTime += (currentTime - previousTime);
  vector<vec3> waypoints = owner->getWaypoints()["start"];

  static int wi = 0;
  const float maxDistance = 150.0;
  glm::vec3 targetPosition(waypoints.at(wi).x, me->position.y, waypoints.at(wi).z);

  dist = glm::distance(targetPosition, me->position);
  glm::vec3 dir = targetPosition - me->position;
  cmd->angles[1] = 90 + (atan2(-dir.x, dir.z) * (180.0 / PI));

  if (dist <= maxDistance) {

    if (wi == waypoints.size() - 1) {
      completed = true;
    }

    wi = (wi + 1) % waypoints.size();
    totalTime = 0;
    cmd->forwardMove = 0;
  } else {
    cmd->forwardMove = 500;
  }

  owner->setRespawned(false);
}

double PatrolGoal::calculateDesirability() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory *memory = owner->getBotMemory();
  double desire = 0.001;
  double tweak = 1.0;
  if (owner->getRespawned()) {
    desire = tweak * owner->getHealth();
  }

  return desire;
}
