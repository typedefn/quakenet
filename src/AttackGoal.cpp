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
  goals.push_back(std::make_unique<SeekGoal>(owner));
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

  double maxDesire = 1;
  Goal *goal = nullptr;
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

  if (targetingSystem->isTargetPresent()) {
    int slot = targetingSystem->getTarget();
    glm::vec3 lastPosition = targetingSystem->getLastRecordedPosition();
    PlayerInfo *target = owner->getPlayerBySlot(slot);
    BotConfig botConfig = owner->getBotConfig();

    if (me->team == target->team) {
      targetingSystem->clearTarget();
      return;
    } 

    // If player is in no fire zone stop attacking
    for (int i = 0; i < botConfig.noFirezones.size(); ++i) {
      float radius = botConfig.noFirezones.at(i).first;
      glm::vec3 origin = botConfig.noFirezones.at(i).second;

      if (glm::distance(origin, lastPosition) <= radius) {
        owner->nullButtons();
        return;
      }
    }    
    //float projVel = glm::dot(target->velocity, target->accel);
    float dt = (float)owner->getTime() - (float)target->time;
    glm::vec3 futureVel = target->velocity;
    glm::vec3 newPosition = lastPosition + futureVel * 0.5f;
    glm::vec3 targetPosition = newPosition;
    glm::vec3 toEnemy = targetPosition - me->position;
    glm::vec3 directionToTarget = glm::normalize(targetPosition - me->position);
    float opposite = toEnemy.y - ((rand() + 10) % 40);
    float hypotonus = glm::distance(targetPosition, me->position);
    float yawAngle = 90 + atan2(-directionToTarget.x, directionToTarget.z) * (180.0 / PI);
    float pitchAngle = asinf(opposite / hypotonus) * (180.0 / PI);

    owner->rotateY(yawAngle);
    owner->rotateX(-pitchAngle);

    int randomJump = (5 > (rand() % 1000)) ? 2 : 3;

    owner->clickButton(1 | randomJump);
  } else {
    owner->nullButtons();
    owner->moveForward(0);
  }
}

double AttackGoal::calculateDesirability() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory *memory = owner->getBotMemory();
  PlayerInfo *me = owner->getMe();
  double desire = 0.001;
  double tweaker = 40.0;

  glm::vec3 targetPosition = targetingSystem->getLastRecordedPosition();
  float dist = glm::distance(targetPosition, me->position);

  if (targetingSystem->isTargetPresent() && targetingSystem->isTargetWithinFov() && dist < owner->getBotConfig().targetDistance) {
    PlayerInfo *target = owner->getPlayerBySlot(targetingSystem->getTarget());
    desire = tweaker * (owner->getHealth() / (dist + 100));
  }

  return desire;
}


