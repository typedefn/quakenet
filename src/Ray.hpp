/*
 * Ray.hpp
 *
 *  Created on: Jun 21, 2020
 *      Author: dervish
 */

#ifndef SRC_RAY_HPP_
#define SRC_RAY_HPP_

#include <glm/glm.hpp>
class Ray {
public:
	Ray() {
	}
	Ray(const glm::vec3 &a, const glm::vec3 &b) {
		this->a = a;
		this->b = b;
	}
	virtual ~Ray() {
	}
	glm::vec3 origin() const {
		return a;
	}
	glm::vec3 direction() const {
		return b;
	}
	glm::vec3 pointAtT(float t) const {
		return a + t * b;
	}
private:
	glm::vec3 a;
	glm::vec3 b;
};

#endif /* SRC_RAY_HPP_ */
