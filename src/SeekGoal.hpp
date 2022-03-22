/*
 * SeekGoal.hpp
 *
 *  Created on: Oct 1, 2021
 *      Author: dervish
 */

#ifndef SEEKGOAL_HPP_
#define SEEKGOAL_HPP_
#include "Common.hpp"
#include "Goal.hpp"

class SeekGoal : public Goal {
public:
  SeekGoal(Bot * owner);
  virtual ~SeekGoal();
  void update();
  double calculateDesirability();
  std::string description() const {
    return "Seeking";
  }
private:
  std::vector<std::unique_ptr<Goal>> goals;
};

#endif /* SEEKGOAL_HPP_ */
