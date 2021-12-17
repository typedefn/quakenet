/*
 * BotMemory.cpp
 *
 *  Created on: Sep 15, 2021
 *      Author: dervish
 */

#include "BotMemory.hpp"
#include "Bot.hpp"

BotMemory::BotMemory(Bot *owner, double memorySpan) {
  this->owner = owner;
  this->memorySpan = memorySpan;
  this->angleToTarget = 0.0f;
}

BotMemory::~BotMemory() {
  // TODO Auto-generated destructor stub
}

list<int> BotMemory::getListOfRecentlySensedEntities() const {
  list<int> entities;

  double currentTime = owner->getTime();

  for (const auto &r : memoryMap) {

    if (r.first == owner->getMe()->slot) {
      continue;
    }

    // if this entity has been updated in the memory recently, add to list.
    if ((currentTime - r.second.lastVisible) < memorySpan) {
      entities.push_back(r.first);
    }
  }

  return entities;
}

void BotMemory::updateVision() {

  int target = owner->getTargetingSystem()->getTarget();

  auto rec = memoryMap.find(target);

  if (rec == memoryMap.end()) {
    MemoryRecord mem = { getLastSensedPosition(target), owner->getTime(), 0, false, false, 0, 0 };
    memoryMap[target] = mem;
  }

  for (auto &r : memoryMap) {
    PlayerInfo *p = owner->getPlayerById(r.first);
    PlayerInfo *m = owner->getMe();

    if (m == p) {
      continue;
    }

    if (!p->active) {
      continue;
    }

    MemoryRecord * mem = &(r.second);

    if (isWithinFov(r.first)) {
      mem->lastSensedPosition = getLastSensedPosition(target);
      mem->timeOpponentVisible += (owner->getTime() - mem->lastVisible);
      mem->lastVisible = owner->getTime();
      mem->timeOutOfFov = 0.0;
      mem->withinFov = true;
    } else {
      mem->timeOutOfFov += (owner->getTime() - mem->lastTimeOutOfFov);
      mem->lastTimeOutOfFov = owner->getTime();
      mem->timeOpponentVisible = 0.0;
      mem->withinFov = false;
    }
  }
}

bool BotMemory::isWithinFov(int id) {
  PlayerInfo *me = owner->getMe();

  if (me == nullptr) {
    return false;
  }

  glm::vec3 targetPosition = getLastSensedPosition(id);
  glm::vec3 facing = glm::normalize(glm::cross(glm::normalize(me->position), glm::vec3(0, 1, 0)));
  glm::vec3 directionToTarget = glm::normalize(targetPosition - me->position);

  float deltaAngle = glm::dot(directionToTarget, facing);

  return deltaAngle >= 0.01;
}

vec3 BotMemory::getLastSensedPosition(int id) {
    PlayerInfo *p = owner->getPlayerById(id);
    return p->position;
}

double BotMemory::getTimeEntityHasBeenVisible(int id) const {
  auto rec = memoryMap.find(id);
  if (rec != memoryMap.end()) {
    return memoryMap.at(id).timeOpponentVisible;
  }

  return 0;
}

double BotMemory::getTimeEntityHasBeenOutOfFov(int id) const {
  auto rec = memoryMap.find(id);
  if (rec != memoryMap.end()) {
    return memoryMap.at(id).timeOutOfFov;
  }

  return 0.0;
}

