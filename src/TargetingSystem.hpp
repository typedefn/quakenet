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

class Bot;

class TargetingSystem {
public:
	TargetingSystem(Bot *owner);
	virtual ~TargetingSystem();

	void update();

	bool isTargetPresent() const;
	bool isTargetWithinFov() const;
	vec3 getLastRecordedPosition() const;
	double getTimeTargetHasBeenVisible() const;
	double getTimeTargetHasBeenOutOfView() const;
	int getTarget() const;
	void clearTarget();
	void setTarget(int t) {
	  currentTarget = t;
	}
private:
	Bot *owner;
	int currentTarget;
};

#endif /* SRC_TARGETINGSYSTEM_HPP_ */
