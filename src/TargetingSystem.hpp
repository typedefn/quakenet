/*
 * TargetingSystem.hpp
 *
 *  Created on: Sep 25, 2021
 *      Author: mastakillah
 */

#ifndef SRC_TARGETINGSYSTEM_HPP_
#define SRC_TARGETINGSYSTEM_HPP_

#include "Common.hpp"
#include "Entity.hpp"
#include "Box.hpp"
#include "Ray.hpp"
#include "BspParser.hpp"
#include "MathUtil.hpp"

class Bot;

class TargetingSystem {
public:
	TargetingSystem(Bot *owner);
	virtual ~TargetingSystem();

	void update();

	bool isTargetPresent() const;
	bool isTargetWithinFov() const;
	glm::vec3 getLastRecordedPosition() const;
	double getTimeTargetHasBeenVisible() const;
	double getTimeTargetHasBeenOutOfView() const;
	int getTarget() const;
	void clearTarget();
	void setTarget(int t) {
	  currentTarget = t;
	}
        void setBspModel(Model bspModel) {
          this->bspModel = bspModel;
        }
private:
	Bot *owner;
	int currentTarget;
        Model bspModel;

  bool targetIsVisible(int id);
};

#endif /* SRC_TARGETINGSYSTEM_HPP_ */
