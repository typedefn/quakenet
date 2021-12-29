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
  goals.push_back(make_unique<SeekGoal>(owner));
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
    vec3 lastPosition = targetingSystem->getLastRecordedPosition();
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

    double previousTime = currentTime;
    currentTime = owner->getTime();

    totalTime += (currentTime - previousTime);
    if (totalTime > 3) {
      sign = Utility::getRandomNormal();
      totalTime = 0;
    }

    size_t frame = (owner->getFrame()) % UPDATE_BACKUP;

    Command * command = &owner->getCommands()[frame];
    command->buttons |= 1;
//    command->buttons |= (Utility::getRandomNormal() * 2);
    command->angles[0] = -pitchAngle;
    command->angles[1] = yawAngle;



    double maxDesire = 2;
    Goal * goal = nullptr;
    for (const auto &g : goals) {
      double currentDesire = g->calculateDesirability();

      if (currentDesire > maxDesire) {
        maxDesire = currentDesire;
        goal = g.get();
      }
    }

    if (goal != nullptr) {
      goal->update();
    }
  }

}

double AttackGoal::calculateDesirability() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory *memory = owner->getBotMemory();
  PlayerInfo *me = owner->getMe();
  double desire = 1.0;
  double tweaker = 1.0;

  if (targetingSystem->isTargetPresent() && targetingSystem->isTargetWithinFov()) {
    int id = targetingSystem->getTarget();
    glm::vec3 targetPosition = targetingSystem->getLastRecordedPosition();
    float dist = glm::distance(targetPosition, me->position);

    desire = tweaker * (owner->getHealth())/dist;
  }

  return desire;
}
