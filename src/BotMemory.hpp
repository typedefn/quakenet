/*
 * BotMemory.hpp
 *
 *  Created on: Sep 15, 2021
 *      Author: dervish
 */

#ifndef BOTMEMORY_HPP_
#define BOTMEMORY_HPP_

#include "Common.hpp"
#include "Bot.hpp"

struct MemoryRecord {
  double lastVisible;
  double timeOpponentVisible;
  glm::vec3 lastSensedPosition;
  bool withinFov;
  bool shootable;
};

class BotMemory {
public:
  BotMemory(Bot *owner, double memorySpan);
  virtual ~BotMemory();

private:
  Bot *owner;

  map<Entity*, MemoryRecord> memoryMap;

  double memorySpan;

  void addOrUpdateMemoryRecord(Entity *entity);
  void updateVision();

  bool isWithinFov(Entity *entity);
  vec3 getLastSensedPosition(Entity *entity) const;
  double getTimeEntityHasBeenVisible(Entity *entity) const;
  double getTimeEntityHasBeenOutOfFov(Entity *entity) const;
  // list of all the entities that had their records updated within the last memorySpan seconds.
  list<Entity*> getListOfRecentlySensedEntities() const;
};

#endif /* BOTMEMORY_HPP_ */
