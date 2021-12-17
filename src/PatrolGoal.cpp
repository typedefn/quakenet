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
  vector<vec3> waypoints = owner->getWaypoints();

  static int wi = 0;
  const float maxDistance = 100.0;
  glm::vec3 targetPosition(waypoints.at(wi).x, me->position.y, waypoints.at(wi).z);

  previousDist = dist;
  dist = glm::distance(targetPosition, me->position);

  if (dist <= maxDistance) {
    wi = (wi + 1) % waypoints.size();
    totalTime = 0;
  }

  glm::vec3 dir = targetPosition - me->position;

  cmd->angles[1] = 90 + (atan2(-dir.x, dir.z) * (180.0 / PI));
  cmd->forwardMove = 500;

  if (totalTime > 5 && (dist - previousDist) < 15.0) {
    wi = (wi + 1) % waypoints.size();
  }
}

double PatrolGoal::calculateDesirability() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory *memory = owner->getBotMemory();
  double desire = 1;
  double tweak = 1;
  if (targetingSystem->isTargetPresent()) {
    int id = targetingSystem->getTarget();
    desire = tweak * owner->getHealth() * memory->getTimeEntityHasBeenOutOfFov(id);
  }

  return desire;
}
