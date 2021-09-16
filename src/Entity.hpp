/*
 * Entity.hpp
 *
 *  Created on: Sep 15, 2021
 *      Author: dervish
 */

#ifndef ENTITY_HPP_
#define ENTITY_HPP_
#include "Protocol.hpp"
#include "Common.hpp"

struct PlayerInfo {
  long userId;
  char name[32];
  int frags;
  int ping;
  byte pl;
  float entertime;
  int slot;
  float coords[3];
  int frame;
  short flags;
};

class Entity {
public:
  Entity();
  virtual ~Entity();
};

#endif /* ENTITY_HPP_ */
