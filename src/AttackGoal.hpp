/*
 * AttackGoal.hpp
 *
 *  Created on: Sep 30, 2021
 *      Author: dervish
 */

#ifndef ATTACKGOAL_HPP_
#define ATTACKGOAL_HPP_
#include "Common.hpp"
#include "Goal.hpp"

class Bot;

class AttackGoal : public Goal {
public:
  AttackGoal(Bot * owner);
  virtual ~AttackGoal();
  void update();
  double calculateDesirability();

private:
  Bot * owner;
};

#endif /* ATTACKGOAL_HPP_ */
