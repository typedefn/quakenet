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
  goals.push_back(make_unique<StrafeGoal>(owner));
}

SeekGoal::~SeekGoal() {
  // TODO Auto-generated destructor stub
}

void SeekGoal::update() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory *memory = owner->getBotMemory();
  PlayerInfo *me = owner->getMe();
  Command *cmd = owner->getCommand();

  if (targetingSystem->isTargetPresent()) {
    const float maxDistance = 400.0;
    glm::vec3 targetPosition = targetingSystem->getLastRecordedPosition();
    float dist = glm::distance(targetPosition, me->position);

    glm::vec3 dir = targetPosition - me->position;

    cmd->angles[1] = 90 + (atan2(-dir.x, dir.z) * (180.0 / PI));
    cmd->forwardMove = 500;


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
