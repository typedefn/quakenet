
/* 
 * File:   Gene.hpp
 * Author: dervish
 *
 * Created on March 6, 2021, 7:34 PM
 */

#ifndef GENE_HPP
#define GENE_HPP
#include "Utility.hpp"

// In Milliseconds
#define MAX_ACTION_DURATION_MS 4000.0f

enum ActionType {
//  ROTATE_LEFT,
//  ROTATE_RIGHT,
  ATTACK,
  WALK,
  WALK_BACK,
  SIDE_LEFT,
  SIDE_RIGHT
};

class Gene {
public:
  Gene();
  Gene(ActionType a, float d);
  virtual ~Gene();

  bool operator==(const Gene &rhs) const {
    return (action == rhs.action) && (duration == rhs.duration);
  }

  float getDuration() const {
    return duration;
  }

  void setDuration(float d) {
    duration = d;
  }

  void setActionType(ActionType a) {
    action = a;
  }

  ActionType getActionType() const {
    return action;
  }

private:

  ActionType action;
  float duration;

};

#endif /* GENE_HPP */

