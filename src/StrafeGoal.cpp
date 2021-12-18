/*
 * StrafeGoal.cpp
 *
 *  Created on: Dec 16, 2021
 *      Author: dervish
 */

#include "StrafeGoal.hpp"
#include "Bot.hpp"

StrafeGoal::StrafeGoal(Bot *owner) {
  // TODO Auto-generated constructor stub
  this->owner = owner;
  currentTime = 0;
  sign = 0;
  totalTime = 0;
  previousDist = 0;
  dist = 0;
}

StrafeGoal::~StrafeGoal() {
  // TODO Auto-generated destructor stub
}

void StrafeGoal::update() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory *memory = owner->getBotMemory();
  PlayerInfo *me = owner->getMe();
  Command *cmd = owner->getCommand();

  double previousTime = currentTime;
  currentTime = owner->getTime();

  totalTime += (currentTime - previousTime);

  if (totalTime > 3) {
    sign = Utility::getRandomNormal();
    totalTime = 0;
  }
  cmd->sideMove = sign * 250;

}

double StrafeGoal::calculateDesirability() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory *memory = owner->getBotMemory();
  PlayerInfo *me = owner->getMe();
  double desire = 0.0;

  if (targetingSystem->isTargetPresent() && targetingSystem->isTargetWithinFov()) {
    int id = targetingSystem->getTarget();
    glm::vec3 targetPosition = targetingSystem->getLastRecordedPosition();
    previousDist = dist;
    dist = glm::distance(targetPosition, me->position);

    double previousTime = currentTime;
    currentTime = owner->getTime();
    totalTime += (currentTime - previousTime);

    if (fabs(previousDist - dist) < 15 && totalTime > 2) {
      desire = (memory->getTimeEntityHasBeenVisible(id) / dist);
    }

  }

  return desire;
}