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
  goals.push_back(std::make_unique<StrafeGoal>(owner));
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

  if (targetingSystem->isTargetPresent()) {
    int slot = targetingSystem->getTarget();
    glm::vec3 lastPosition = targetingSystem->getLastRecordedPosition();
    float targetSpeed = owner->getPlayerBySlot(slot)->speed;

    glm::vec3 toEnemy = lastPosition - me->position;
    float lookAhead = glm::length(toEnemy) / (targetSpeed + 6.0);

    glm::vec3 velocity = owner->getPlayerBySlot(slot)->velocity;
    glm::vec3 targetPosition = lastPosition + velocity * lookAhead;

    glm::vec3 directionToTarget = glm::normalize(targetPosition - me->position);

    float opposite = toEnemy.y - 40;
    float hypotonus = glm::distance(lastPosition, me->position);

    float yawAngle = 90 + atan2(-directionToTarget.x, directionToTarget.z) * (180.0 / PI);
    float pitchAngle = asinf(opposite / hypotonus) * (180.0 / PI);

    owner->rotateY(yawAngle);
    owner->rotateX(-pitchAngle);
    owner->clickButton(1 | ((rand() % 2) * 2));
  }

  double maxDesire = -10;
  Goal *goal = nullptr;
  for (const auto &g : goals) {
//    double currentDesire = g->calculateDesirability();
//    if (currentDesire > maxDesire) {
//      maxDesire = currentDesire;
//      goal = g.get();
//    }

    g->update();
  }
//
//  if (goal != nullptr) {
//    goal->update();
//  }
}

double AttackGoal::calculateDesirability() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory *memory = owner->getBotMemory();
  PlayerInfo *me = owner->getMe();
  double desire = 0.0;
  double tweaker = 40.0;

  if (targetingSystem->isTargetPresent() && targetingSystem->isTargetWithinFov()) {
    glm::vec3 targetPosition = targetingSystem->getLastRecordedPosition();
    float dist = glm::distance(targetPosition, me->position);

    desire = tweaker * (owner->getHealth() / (dist + 100));
  }

  return desire;
}
