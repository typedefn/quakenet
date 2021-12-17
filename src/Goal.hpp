/*
 * Goal.hpp
 *
 *  Created on: Sep 30, 2021
 *      Author: dervish
 */

#ifndef GOAL_HPP_
#define GOAL_HPP_
#include "Common.hpp"
class Bot;

class Goal {
public:
  Goal() {};
  virtual ~Goal() {}
  virtual void update() = 0;
  virtual double calculateDesirability() = 0;
  virtual string description() const = 0;

  bool isCompleted() {
    return completed;
  }

protected:
  Bot *owner;
  double totalTime;
  double currentTime;
  int sign;
  bool completed;

  vector<unique_ptr<Goal>> goals;
};



#endif /* GOAL_HPP_ */
