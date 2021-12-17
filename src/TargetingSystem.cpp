/*
 * TargetingSystem.cpp
 *
 *  Created on: Sep 25, 2021
 *      Author: mastakillah
 */

#include "TargetingSystem.hpp"
#include "Bot.hpp"

TargetingSystem::TargetingSystem(Bot *owner) {
	// TODO Auto-generated constructor stub
	this->owner = owner;
	currentTarget = -1;
}

TargetingSystem::~TargetingSystem() {
	// TODO Auto-generated destructor stub
}

void TargetingSystem::update() {
	double closestDistanceSoFar = 999999999.0;
	currentTarget = owner->getTargetId();

	list<int> entities =
			owner->getBotMemory()->getListOfRecentlySensedEntities();

	for (const auto id : entities) {
		PlayerInfo *p = owner->getPlayerById(id);
		PlayerInfo *m = owner->getMe();

		if (p == m) {
			continue;
		}

		if (!p->active) {
			continue;
		}

		float dist = glm::distance(owner->getBotMemory()->getLastSensedPosition(id), m->position);

		if (dist < closestDistanceSoFar && dist <= 800) {
			closestDistanceSoFar = dist;
			currentTarget = id;
		}
	}
}

bool TargetingSystem::isTargetPresent() const {
	return currentTarget != -1 && currentTarget != owner->getMe()->slot;
}

bool TargetingSystem::isTargetWithinFov() const {
	return owner->getBotMemory()->isWithinFov(currentTarget);
}

vec3 TargetingSystem::getLastRecordedPosition() const {
	return owner->getBotMemory()->getLastSensedPosition(currentTarget);
}

double TargetingSystem::getTimeTargetHasBeenVisible() const {
	return owner->getBotMemory()->getTimeEntityHasBeenVisible(currentTarget);
}

double TargetingSystem::getTimeTargetHasBeenOutOfView() const {
	return owner->getBotMemory()->getTimeEntityHasBeenOutOfFov(currentTarget);
}

int TargetingSystem::getTarget() const {
	return currentTarget;
}

void TargetingSystem::clearTarget() {
	currentTarget = -1;
}
