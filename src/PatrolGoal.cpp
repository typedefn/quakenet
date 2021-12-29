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
  currentTime = 0;
  totalTime = 0;
  previousDist = 0;
  dist = 0;
  frame = owner->getFrame();
  prevFrame = 0;

  Command *command = owner->getCommand();
}

PatrolGoal::~PatrolGoal() {
  // TODO Auto-generated destructor stub
}

void PatrolGoal::update() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory *memory = owner->getBotMemory();
  PlayerInfo *me = owner->getMe();

  double previousTime = currentTime;
  currentTime = owner->getTime();

  totalTime += (currentTime - previousTime);
  vector<vec3> waypoints = owner->getWaypoints()["start"];

  static int wi = 0;
  const float maxDistance = 200.0;
  glm::vec3 targetPosition(waypoints.at(wi).x, me->position.y, waypoints.at(wi).z);

  dist = glm::distance(targetPosition, me->position);
  glm::vec3 dir = targetPosition - me->position;
  float yaw = 90 + (atan2(-dir.x, dir.z) * (180.0 / PI));

  Command *commands = owner->getCommands();
  int frame = (owner->getFrame() + 2) % UPDATE_BACKUP;
  commands[frame].forwardMove = 0;

  if (dist <= maxDistance) {
    commands[frame].forwardMove = 0;
  } else {
    commands[frame].forwardMove = 280;
  }

  commands[frame].buttons = 0;
  commands[frame].angles[1] = yaw;
}

double PatrolGoal::calculateDesirability() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory *memory = owner->getBotMemory();
  double desire = .01;
  double tweak = 1.0;
  if (!targetingSystem->isTargetPresent() && !targetingSystem->isTargetWithinFov()) {
    desire = tweak * owner->getHealth();
  }
  return desire;
}
