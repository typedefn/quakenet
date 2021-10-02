/*
 * Goal.hpp
 *
 *  Created on: Sep 30, 2021
 *      Author: dervish
 */

#ifndef GOAL_HPP_
#define GOAL_HPP_

class Bot;

class Goal {
public:
  Goal() {};
  virtual ~Goal() {}
  virtual void update() = 0;
  virtual double calculateDesirability() = 0;
protected:
  Bot *owner;
};



#endif /* GOAL_HPP_ */
