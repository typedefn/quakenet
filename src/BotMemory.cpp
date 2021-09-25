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
}

BotMemory::~BotMemory() {
	// TODO Auto-generated destructor stub
}

list<int> BotMemory::getListOfRecentlySensedEntities() const {
	list<int> entities;

	double currentTime = owner->getTime();

	for (const auto &r : memoryMap) {
		// if this entity has been updated in the memory recently, add to list.
		if ((currentTime - r.second.lastVisible) < memorySpan) {
			entities.push_back(r.first);
		}
	}

	return entities;
}

void BotMemory::updateVision() {
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
	glm::vec3 position(me->coords[0], me->coords[2], me->coords[1]);
	glm::vec3 facing = glm::normalize(
			glm::cross(glm::normalize(position), glm::vec3(0, 1, 0)));
	glm::vec3 directionToTarget = glm::normalize(targetPosition - position);

	float deltaAngle = glm::dot(directionToTarget, facing);

	return deltaAngle >= 0.10;
}

vec3 BotMemory::getLastSensedPosition(int id) const {
	PlayerInfo *p = owner->getPlayerById(id);
	glm::vec3 targetPosition(p->coords[0], p->coords[2], p->coords[1]);
	return targetPosition;
}

double BotMemory::getTimeEntityHasBeenVisible(int id) const {
	auto rec = memoryMap.find(id);
	if ( rec != memoryMap.end()) {
		return memoryMap.at(id).timeOpponentVisible;
	}

	return 0.0;
}

double BotMemory::getTimeEntityHasBeenOutOfFov(int id) const {
	auto rec = memoryMap.find(id);
	if ( rec != memoryMap.end()) {
		return memoryMap.at(id).timeOutOfFov;
	}

	return 9999999999.0;
}

