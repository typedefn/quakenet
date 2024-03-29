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
  glm::vec3 lastSensedPosition;
	double lastVisible;
	double timeOpponentVisible;
	bool withinFov;
	bool shootable;
	double timeOutOfFov;
	double lastTimeOutOfFov;
	int slot;
};

class BotMemory {
public:
	BotMemory(Bot *owner, double memorySpan);
	virtual ~BotMemory();
	void updateVision();

private:
	Bot *owner;

	std::map<int, MemoryRecord> memoryMap;

	double memorySpan;
	double angleToTarget;

public:
	bool isWithinFov(int id);
	glm::vec3 getLastSensedPosition(int id);
	double getTimeEntityHasBeenVisible(int id) const;
	double getTimeEntityHasBeenOutOfFov(int id) const;
	// list of all the entities that had their records updated within the last memorySpan seconds.
	std::list<int> getListOfRecentlySensedEntities();
};

#endif /* BOTMEMORY_HPP_ */
