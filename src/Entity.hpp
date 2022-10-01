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
  std::string name;
  int frags;
  int ping;
  byte pl;
  float entertime;
  int slot;
  float coords[3];
  int frame;
  int flags;
  bool active;
  glm::vec3 velocity;
  glm::vec3 position;
  double time;
  float speed;
  glm::vec3 direction;
  float angles[3];
  std::string team;
};

#endif /* ENTITY_HPP_ */
