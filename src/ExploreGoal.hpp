/*
 * SeekGoal.hpp
 *
 *  Created on: Oct 1, 2021
 *      Author: dervish
 */

#ifndef _EXPLORE_GOAL_HPP_
#define _EXPLORE_GOAL_HPP_
#include "Common.hpp"
#include "Goal.hpp"

class ExploreGoal : public Goal {
public:
  ExploreGoal(Bot * owner);
  virtual ~ExploreGoal();
  void update();
  double calculateDesirability();
  std::string description() const {
    return "exploring";
  }
private:
  bool pathIsVisible(glm::vec3 targetPosition, float * dist);
  glm::mat4 rot;
  glm::vec3 forward;
  double timer;
  bool visible;
  float myYaw;
  glm::vec3 destination;
  float scanRange;
  glm::vec3 mainHeading;
  bool movement;
  glm::vec3 gotoPosition;
  int hitSensor;
};

#endif
