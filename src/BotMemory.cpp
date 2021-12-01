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

  auto rec = memoryMap.find(owner->getTargetId());

  if ( rec == memoryMap.end()) {
    MemoryRecord mem;
    mem.lastVisible = owner->getTime();
    memoryMap[owner->getTargetId()] = mem;
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

		MemoryRecord mem;

		if (isWithinFov(r.first)) {
			glm::vec3 targetPosition = getLastSensedPosition(r.first);
			mem.lastSensedPosition = targetPosition;
			double prevLast = r.second.lastVisible;
			mem.lastVisible = owner->getTime();
			mem.timeOpponentVisible += (r.second.lastVisible - prevLast);
			mem.withinFov = true;
		} else {
			double last = r.second.lastTimeOutOfFov;
			mem.lastTimeOutOfFov = owner->getTime();
			mem.timeOutOfFov += (r.second.lastTimeOutOfFov - last);
		}

		memoryMap[r.first] = mem;
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
  cout << "deltaAngle = " << deltaAngle << endl;
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

  return 0.0;
}

double BotMemory::getTimeEntityHasBeenOutOfFov(int id) const {
  auto rec = memoryMap.find(id);
  if (rec != memoryMap.end()) {
    return memoryMap.at(id).timeOutOfFov;
  }

  return 9999999999.0;
}

