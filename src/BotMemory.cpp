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

list<int> BotMemory::getListOfRecentlySensedEntities() {
  list<int> entities;

  double currentTime = owner->getTime();

  for (const pair<int, MemoryRecord> r : memoryMap) {
    // if this entity has been updated in the memory recently, add to list.
    double delta = (currentTime - r.second.lastVisible);
    if (delta < memorySpan) {
      entities.push_back(r.second.slot);
    }
  }

  return entities;
}

void BotMemory::updateVision() {
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

    if (isWithinFov(num)) {
      m.lastSensedPosition = pi->position;
      m.timeOpponentVisible += (owner->getTime() - m.lastVisible);
      m.lastVisible = owner->getTime();
      m.timeOutOfFov = 0.0;
      m.withinFov = true;
    } else {
      m.timeOutOfFov += (owner->getTime() - m.lastTimeOutOfFov);
      m.lastTimeOutOfFov = owner->getTime();
      m.timeOpponentVisible = 0.0;
      m.withinFov = false;
    }

    m.slot = num;
    if(memoryMap.find(num) != memoryMap.end()) {
      memoryMap.erase(num);
    }

    memoryMap.insert(make_pair(num, m));
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
  inFov = (d >= 0.01);

//  LOG << " in fov = " << inFov << " d = " << d << " t.x " << target.x << " t.z " << target.z;
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

