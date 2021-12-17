/*
 * StrafeGoal.hpp
 *
 *  Created on: Dec 16, 2021
 *      Author: dervish
 */

#ifndef STRAFEGOAL_HPP_
#define STRAFEGOAL_HPP_
#include "Common.hpp"
#include "Goal.hpp"

class Bot;

class StrafeGoal : public Goal {
public:
  StrafeGoal(Bot * owner);
  virtual ~StrafeGoal();
  void update();
  double calculateDesirability();
  string description() const {
    return "Strafing";
  }
private:
  double totalTime;
  double currentTime;
  int sign;
};

#endif /* STRAFEGOAL_HPP_ */
