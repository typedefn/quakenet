/*
 * Goal.hpp
 *
 *  Created on: Sep 30, 2021
 *      Author: dervish
 */

#ifndef GOAL_HPP_
#define GOAL_HPP_

class Goal {
public:
  Goal() {};
  virtual ~Goal() {}
  virtual void update() = 0;
  virtual double calculateDesirability() = 0;
};



#endif /* GOAL_HPP_ */
