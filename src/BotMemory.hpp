/*
 * BotMemory.hpp
 *
 *  Created on: Sep 15, 2021
 *      Author: dervish
 */

#ifndef BOTMEMORY_HPP_
#define BOTMEMORY_HPP_

#include "Common.hpp"
#include "Entity.hpp"

class Bot;
struct MemoryRecord {
	double lastVisible;
	double timeOpponentVisible;
	glm::vec3 lastSensedPosition;
	bool withinFov;
	bool shootable;
	double timeOutOfFov;
	double lastTimeOutOfFov;
};

class BotMemory {
public:
	BotMemory(Bot *owner, double memorySpan);
	virtual ~BotMemory();
	void updateVision();

private:
	Bot *owner;

	map<int, MemoryRecord> memoryMap;

	double memorySpan;
	double angleToTarget;

public:

	float getAngleToTarget() { return angleToTarget; }
	bool isWithinFov(int id);
	vec3 getLastSensedPosition(int id);
	double getTimeEntityHasBeenVisible(int id) const;
	double getTimeEntityHasBeenOutOfFov(int id) const;
	// list of all the entities that had their records updated within the last memorySpan seconds.
	list<int> getListOfRecentlySensedEntities() const;
};

#endif /* BOTMEMORY_HPP_ */
