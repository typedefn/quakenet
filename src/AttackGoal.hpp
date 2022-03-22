/*
 * AttackGoal.hpp
 *
 *  Created on: Sep 30, 2021
 *      Author: dervish
 */

#ifndef ATTACKGOAL_HPP_
#define ATTACKGOAL_HPP_

#include "Goal.hpp"
#include "StrafeGoal.hpp"
class Bot;

class AttackGoal : public Goal {
public:
  AttackGoal(Bot * owner);
  virtual ~AttackGoal();
  void update();
  double calculateDesirability();
  std::string description() const {
    return "Attacking";
  }
private:

};

#endif /* ATTACKGOAL_HPP_ */
