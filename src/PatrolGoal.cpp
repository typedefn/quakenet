/*
 * PatrolGoal.cpp
 *
 *  Created on: Sep 30, 2021
 *      Author: dervish
 */

#include "PatrolGoal.hpp"
#include "Bot.hpp"

PatrolGoal::PatrolGoal(Bot *owner) {
  // TODO Auto-generated constructor stub
  this->owner = owner;
}

PatrolGoal::~PatrolGoal() {
  // TODO Auto-generated destructor stub
}

void PatrolGoal::update() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory *memory = owner->getBotMemory();
  PlayerInfo * me = owner->getMe();
  Command *cmd = owner->getCommand();

  if (!targetingSystem->isTargetPresent() || !targetingSystem->isTargetWithinFov()) {
    vector<glm::vec3> waypoints = owner->getWaypoints();

    static int wi = 0;
    const float maxDistance = 100.0;
    glm::vec3 targetPosition(waypoints.at(wi).x, me->position.y,
        waypoints.at(wi).z);

    float dist = glm::distance(targetPosition, me->position);

    if (dist <= maxDistance) {
      wi = (wi + 1) % waypoints.size();
    }

    glm::vec3 dir = targetPosition - me->position;

    cmd->angles[1] = 90 + (atan2(-dir.x, dir.z) * (180.0 / PI));
    cmd->forwardMove = 500;
  }
}

double PatrolGoal::calculateDesirability() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory *memory = owner->getBotMemory();
  double desire = 0.2;

  if (targetingSystem->isTargetPresent()) {
    int id = targetingSystem->getTarget();
    desire = memory->getTimeEntityHasBeenOutOfFov(id);
  }

  LOG << "Patrol goal desire " << desire;

  return desire;
}
