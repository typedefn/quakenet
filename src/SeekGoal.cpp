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

  const int maxDistance = 400.0;
  glm::vec3 targetPosition = targetingSystem->getLastRecordedPosition();

  int dist = glm::distance(targetPosition, me->position);
  glm::vec3 dir = targetPosition - me->position;
  float yaw = 90 + (atan2(-dir.x, dir.z) * (180.0 / PI));

  if (dist > maxDistance) {
    owner->moveForward(glm::min(248, dist+50));
  }

  owner->rotateY(yaw);
}

double SeekGoal::calculateDesirability() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory *memory = owner->getBotMemory();
  PlayerInfo *me = owner->getMe();

  double desire = 0.1;
  double tweak = 2;

  if (targetingSystem->isTargetPresent()) {
    int id = targetingSystem->getTarget();
    glm::vec3 targetPosition = targetingSystem->getLastRecordedPosition();
    float dist = glm::distance(me->position, targetPosition);
    desire = (tweak*owner->getHealth())/(owner->getHealth()+dist-400);
  }

  return desire;
}
