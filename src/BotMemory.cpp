/*
 * BotMemory.cpp
 *
 *  Created on: Sep 15, 2021
 *      Author: dervish
 */

#include "BotMemory.hpp"

BotMemory::BotMemory(Bot *owner, double memorySpan) {
  this->owner = owner;
  this->memorySpan = memorySpan;
}

BotMemory::~BotMemory() {
  // TODO Auto-generated destructor stub
}

list<Entity*> BotMemory::getListOfRecentlySensedEntities() const {
  list<Entity*> entities;

  double currentTime = owner->getTime();

  for(auto r : memoryMap) {
    // if this entity has been updated in the memory recently, add to list.
    if( (currentTime - r.second.lastVisible) < memorySpan) {
      entities.push_back(r.first);
    }
  }

  return entities;
}
