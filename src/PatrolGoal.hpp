/*
 * PatrolGoal.hpp
 *
 *  Created on: Sep 30, 2021
 *      Author: dervish
 */

#ifndef PATROLGOAL_HPP_
#define PATROLGOAL_HPP_

#include "Common.hpp"
#include "Goal.hpp"

class Bot;

class PatrolGoal: public Goal {
public:
  PatrolGoal(Bot *owner);
  virtual ~PatrolGoal();
  void update();
  double calculateDesirability();

private:

};

#endif /* PATROLGOAL_HPP_ */
