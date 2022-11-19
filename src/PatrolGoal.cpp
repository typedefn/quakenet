/*
 * PatrolGoal.cpp
 *
 *  Created on: Sep 30, 2021
 *      Author: dervish
 */

#include "PatrolGoal.hpp"
#include "Bot.hpp"

PatrolGoal::PatrolGoal(Bot *owner) {
  this->owner = owner;
  currentTime = 0;
  totalTime = 0;
  dist = 0;
  wi = 0;
}

PatrolGoal::~PatrolGoal() {
}

void PatrolGoal::update() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory *memory = owner->getBotMemory();
  PlayerInfo *me = owner->getMe();

  BotConfig botConfig = owner->getBotConfig();

  if (!targetingSystem->isTargetPresent()) {
    owner->nullButtons();
  }

  float maxDistance = 230.0;
  dist = 999999;
  float minDist = 99999;
  if (wi == 0) {
    std::string startPoint = "n/a"; 
    // figure out which respawn point we are closest to.
    int numberOfRespawnPoints = botConfig.numRespawns;
    std::string defendPoint = botConfig.defend;

    for(int i = 0; i < numberOfRespawnPoints; i++) {
      std::stringstream respawnSs;
      respawnSs << "respawn" << i;

      if (botConfig.waypoints.find(respawnSs.str()) == botConfig.waypoints.end()) {
	LOG << "respawn section " << respawnSs.str() << " not found in waypoints map";
	return;
      }

      glm::vec3 respawnPoint = botConfig.waypoints[respawnSs.str()].at(0);
      dist = glm::distance(respawnPoint, me->position);

      if (minDist > dist && dist < maxDistance) {
	minDist = dist; 
	startPoint = respawnSs.str();
      }      
    } 

    if (startPoint == "n/a") {
      LOG << "respawn not found!";
      return;
    }
   
    // Get all the waypoints from respawn point to defend point.
    std::stringstream sectionName;
    sectionName << startPoint << "-" << defendPoint;

    if (botConfig.waypoints.find(sectionName.str()) == botConfig.waypoints.end()) {
      LOG << "section " << sectionName.str() << " not found in waypoints map";
      return;
    }

    waypoints = botConfig.waypoints[sectionName.str()];
  } 

  maxDistance = 100.0;
  glm::vec3 targetPosition = waypoints.at(0);

  if (wi < waypoints.size()) {
    targetPosition = glm::vec3(waypoints.at(wi).x, me->position.y, waypoints.at(wi).z);
  }

  dist = glm::distance(targetPosition, me->position);

  if ( wi < waypoints.size() && dist < maxDistance) {
    wi++;
  }

  if (wi == waypoints.size()) {
    finished = true;
    wi = 0;
    return;
  } 

  glm::vec3 dir = normalize(targetPosition - me->position);
  float yaw = 90 + atan2(-dir.x, dir.z) * (180.0 / PI);

  if (dist > maxDistance) {
    owner->moveForward(glm::min(248, dist+50));
  } else {
    owner->moveForward(0);
  }

  owner->rotateY(yaw);
  owner->rotateX(0);
}

double PatrolGoal::calculateDesirability() {
  TargetingSystem *targetingSystem = owner->getTargetingSystem();
  BotMemory *memory = owner->getBotMemory();
  double desire = 0.001;
  double tweak = 1.0;
  if (!targetingSystem->isTargetPresent()) {
    desire = tweak * owner->getHealth()/100.0f;
  }
  return desire;
}

void PatrolGoal::reset() {
  finished = false;
  wi = 0;
}
