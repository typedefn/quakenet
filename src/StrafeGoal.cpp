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
    int r = ((rand() % 3));

    switch (r) {
    case 0:
      sign = 1;
      break;
    case 1:
      sign = -1;
      break;
    default:
      sign = 0;
    }
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
    float dist = glm::distance(targetPosition, me->position);

    desire = (memory->getTimeEntityHasBeenVisible(id) / dist) * 500.0;
  }

  LOG << "Strafe desire " << desire;
  return desire;
}
