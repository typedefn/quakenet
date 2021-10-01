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
  BotMemory * memory = owner->getBotMemory();

  Command * cmd = owner->getCommand();

  if (targetingSystem->isTargetPresent() && targetingSystem->isTargetWithinFov()) {
    cmd->angles[1] = memory->getAngleToTarget();
    cmd->forwardMove = 0;
    cmd->buttons = 1;
  }
}


double AttackGoal::calculateDesirability() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory * memory = owner->getBotMemory();
  double desire = 0.0;

  if(targetingSystem->isTargetPresent()) {
    int id = targetingSystem->getTarget();
    desire = memory->getTimeEntityHasBeenVisible(id);
  }

  cout << "Attack goal desire " << desire << endl;

  return desire;
}
