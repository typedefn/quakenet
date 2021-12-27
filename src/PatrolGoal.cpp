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

  double previousTime = currentTime;
  currentTime = owner->getTime();

  totalTime += (currentTime - previousTime);
  vector<vec3> waypoints = owner->getWaypoints()["start"];

  static int wi = 0;
  const float maxDistance = 200.0;
  glm::vec3 targetPosition(waypoints.at(wi).x, me->position.y, waypoints.at(wi).z);

  dist = glm::distance(targetPosition, me->position);
  glm::vec3 dir = targetPosition - me->position;
  float yaw = 90 + (atan2(-dir.x, dir.z) * (180.0 / PI));

  Command * command = owner->getCommand();

  if (dist <= maxDistance) {
    owner->setRespawned(false);
    wi = (wi + 1) % waypoints.size();
    command->forwardMove = 0;
  } else {
    double forwardSpeed = 2 * dist;
    command->forwardMove = glm::min(forwardSpeed, 250.0);
    command->angles[1] = yaw;
  }

  if (totalTime > 1) {
    LOG << " moving " << " dist " << dist << " forward = " << command->forwardMove;
    totalTime = 0;
  }

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
