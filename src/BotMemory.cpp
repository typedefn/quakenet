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

  PlayerInfo *m = owner->getMe();

  for (size_t num = 0; num < MAX_CLIENTS; num++) {
    PlayerInfo *pi = owner->getPlayerBySlot(num);
    if (!pi->active) {
      continue;
    }

    if (num == m->slot) {
      continue;
    }

    MemoryRecord m;
    MemoryRecord * mem = &m;

    if (isWithinFov(num)) {
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

    memoryMap[num] = m;
  }

}

bool BotMemory::isWithinFov(int id) {
  PlayerInfo *me = owner->getMe();
  bool inFov = false;
  if (me == nullptr) {
    return false;
  }
  vec3 position = me->position;

  vec3 target = owner->getTargetingSystem()->getLastRecordedPosition();
  vec3 facing = normalize(me->direction);
  double fov = 90;
  vec3 toTarget = normalize(target - position);
  float d = dot(facing, toTarget);
  double c = cos(fov / 2.0);
  inFov = (d >= 0.1);
  return inFov;
}

vec3 BotMemory::getLastSensedPosition(int id) {
  PlayerInfo *p = owner->getPlayerBySlot(id);
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

