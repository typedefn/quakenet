/*
 * AttackGoal.cpp
 *
 *  Created on: Sep 30, 2021
 *      Author: dervish
 */

#include "AttackGoal.hpp"
#include "Bot.hpp"

AttackGoal::AttackGoal(Bot *owner) {
  // TODO Auto-generated constructor stub
  this->owner = owner;
}

AttackGoal::~AttackGoal() {
  // TODO Auto-generated destructor stub
}

void AttackGoal::update() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory *memory = owner->getBotMemory();
  PlayerInfo *me = owner->getMe();
  Command *cmd = owner->getCommand();

  if (targetingSystem->isTargetPresent() && targetingSystem->isTargetWithinFov()) {
    vec3 lastPosition = targetingSystem->getLastRecordedPosition();
    int id = targetingSystem->getTarget();
    glm::vec3 toEnemy = lastPosition - me->position;
    float lookAhead = glm::length(toEnemy)/(1000.0f);

    glm::vec3 velocity = owner->getPlayerById(id)->velocity;
    glm::vec3 targetPosition = lastPosition + velocity * lookAhead;

    glm::vec3 facing = glm::normalize(glm::cross(glm::normalize(me->position), glm::vec3(0, 1, 0)));

    glm::vec3 directionToTarget = glm::normalize(targetPosition - me->position);

    float opposite = toEnemy.y - 20;
    float hypotonus = glm::distance(lastPosition, me->position);

    float angleToTarget = 90 + (atan2f(-directionToTarget.x, directionToTarget.z) * (180.0 / PI));
    float angleY = asinf(opposite/hypotonus) * (180.0 / PI);
    float deltaAngle = glm::dot(directionToTarget, facing);

    cmd->angles[1] = angleToTarget;
    cmd->angles[0] = -angleY;
    cmd->forwardMove = 0;
    cmd->buttons = 1;
  }
}

double AttackGoal::calculateDesirability() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory *memory = owner->getBotMemory();
  double desire = 0.0;

  if (targetingSystem->isTargetPresent()) {
    int id = targetingSystem->getTarget();
    desire = memory->getTimeEntityHasBeenVisible(id);
  }

  cout << "Attack goal desire " << desire << endl;

  return desire;
}
