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
  Goal() { sign = 0; owner = nullptr; currentTime = 0; totalTime = 0; finished = false; }
  virtual ~Goal() {}
  virtual void update() = 0;
  virtual double calculateDesirability() = 0;
  virtual std::string description() const = 0;
  virtual void reset() { finished = false; }

  virtual bool isFinished() { return finished; }
protected:
  Bot *owner;
  double totalTime;
  double currentTime;
  int sign;
  bool finished;

  std::vector<std::unique_ptr<Goal>> goals;
};



#endif /* GOAL_HPP_ */
