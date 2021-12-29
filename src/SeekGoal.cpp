/*
 * SeekGoal.cpp
 *
 *  Created on: Oct 1, 2021
 *      Author: dervish
 */

#include "SeekGoal.hpp"
#include "Bot.hpp"
#include "AttackGoal.hpp"

SeekGoal::SeekGoal(Bot *owner) {
  // TODO Auto-generated constructor stub
  this->owner = owner;
}

SeekGoal::~SeekGoal() {
  // TODO Auto-generated destructor stub
}

void SeekGoal::update() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory *memory = owner->getBotMemory();
  PlayerInfo *me = owner->getMe();

  if (targetingSystem->isTargetPresent()) {
    const float maxDistance = 400.0;
    glm::vec3 targetPosition = targetingSystem->getLastRecordedPosition();
    float dist = glm::distance(targetPosition, me->position);

    glm::vec3 dir = targetPosition - me->position;

    size_t frame = (owner->getFrame()) % UPDATE_BACKUP;
    Command *command = &owner->getCommands()[frame];

//    command->angles[1] = 90 + (atan2(-dir.x, dir.z) * (180.0 / PI));
    command->forwardMove = 250;
  }

}

double SeekGoal::calculateDesirability() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory *memory = owner->getBotMemory();
  PlayerInfo *me = owner->getMe();

  double desire = 0.0;
  double tweak = 1.0;
  // TODO: Get max health from server.
  double maxHealth = 100;
  if (targetingSystem->isTargetPresent() && !targetingSystem->isTargetWithinFov() && owner->getHealth() > 0) {
    int id = targetingSystem->getTarget();
    glm::vec3 targetPosition = targetingSystem->getLastRecordedPosition();
    float dist = glm::distance(me->position, targetPosition);
    desire = tweak * ((maxHealth - owner->getHealth())/dist);
  }

  return desire;
}
