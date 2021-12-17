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
  goals.push_back(make_unique<StrafeGoal>(owner));
  totalTime = 0.0;
  sign = 0;
  currentTime = 0;
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
    int id = targetingSystem->getTarget();
    vec3 lastPosition = targetingSystem->getLastRecordedPosition();
    float targetSpeed = owner->getPlayerById(id)->speed;

    glm::vec3 toEnemy = lastPosition - me->position;
    float lookAhead = glm::length(toEnemy) / (targetSpeed + 250);

    glm::vec3 velocity = owner->getPlayerById(id)->velocity;
    glm::vec3 targetPosition = lastPosition + velocity * lookAhead;

    glm::vec3 facing = glm::normalize(glm::cross(glm::normalize(me->position), glm::vec3(0, 1, 0)));

    glm::vec3 directionToTarget = glm::normalize(targetPosition - me->position);

    float opposite = toEnemy.y - 20;
    float hypotonus = glm::distance(lastPosition, me->position);

    float yawAngle = 90 + (atan2(-directionToTarget.x, directionToTarget.z) * (180.0 / PI));
    float pitchAngle = asinf(opposite / hypotonus) * (180.0 / PI);
    float deltaAngle = glm::dot(directionToTarget, facing);

    double previousTime = currentTime;
    currentTime = owner->getTime();

    totalTime += (currentTime - previousTime);

    if (totalTime > 3) {
      sign = Utility::getRandomNormal();
      totalTime = 0;
    }

    cmd->angles[1] = yawAngle;
    cmd->angles[0] = -pitchAngle;
    cmd->forwardMove = sign;
    cmd->buttons = 1;
  }

  for (const auto &g : goals) {
    g->update();
  }

  completed = true;
}

double AttackGoal::calculateDesirability() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory *memory = owner->getBotMemory();
  PlayerInfo *me = owner->getMe();
  double desire = 0.0;
  double tweaker = 1.0;

  if (targetingSystem->isTargetPresent() && targetingSystem->isTargetWithinFov()) {
    int id = targetingSystem->getTarget();
    glm::vec3 targetPosition = targetingSystem->getLastRecordedPosition();
    float dist = glm::distance(targetPosition, me->position);

    desire = tweaker * (owner->getHealth())/dist;
  }

  return desire;
}
