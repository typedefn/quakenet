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
  goals.push_back(make_unique<AttackGoal>(owner));
}

SeekGoal::~SeekGoal() {
  // TODO Auto-generated destructor stub
}

void SeekGoal::update() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory *memory = owner->getBotMemory();
  PlayerInfo *me = owner->getMe();
  Command *cmd = owner->getCommand();

  for (const auto &g : goals) {
    g->update();
  }

  if (targetingSystem->isTargetPresent()) {
    const float maxDistance = 400.0;
    glm::vec3 targetPosition = targetingSystem->getLastRecordedPosition();

    float dist = glm::distance(targetPosition, me->position);

    glm::vec3 dir = targetPosition - me->position;

    cmd->angles[1] = 90 + (atan2(-dir.x, dir.z) * (180.0 / PI));
    cmd->forwardMove = 500;
  }
}

double SeekGoal::calculateDesirability() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory *memory = owner->getBotMemory();
  double desire = 0.0;

  if (targetingSystem->isTargetPresent()) {
    int id = targetingSystem->getTarget();
    desire = owner->getBlood() / 100.0f;
  }

  cout << "Seek goal desire " << desire << endl;

  return desire;
}
