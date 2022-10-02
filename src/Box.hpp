/*
 * Box.hpp
 *
 *  Created on: Oct 16, 2020
 *      Author: dervish
 */

#ifndef SRC_BOX_HPP_
#define SRC_BOX_HPP_
#include <Common.hpp>
#include <Ray.hpp>

class Box {
public:
  Box();
	Box(const glm::vec3 &min, const glm::vec3 &max);
	virtual ~Box();

	bool intersect(const Ray & r, float t0, float t1) const;
	bool intersectV2(const Ray &r, float *t);
        bool intersect(const Ray & r) const;

        bool contains(const Box & b) const;
	void setCenter(glm::vec3 center) {
	  this->center = center;
	}

	glm::vec3 getCenter() const {
	  return this->center;
	}

	void setNormal(glm::vec3 normal) {
	  this->normal = normal;
	}

	glm::vec3 getNormal() {
	  return this->normal;
	}

	void setExra(int value) {
	  this->extra = value;
	}

	int getExtra() {
	  return extra;
	}

	void setKey(std::string key) {
	  this->key = key;
	}

	std::string getKey() {
	  return key;
	}

	glm::vec3 getMin() const {
	  return bounds[0];
	}

	glm::vec3 getMax() const {
	  return bounds[1];
	}

  void setMin(glm::vec3 min) {
    bounds[0] = min;
  }

  void setMax(glm::vec3 max) {
    bounds[1] = max;
  }
private:
	glm::vec3 bounds[2];
	glm::vec3 center;
	glm::vec3 normal;
	std::string key;
  int extra;
};

#endif /* SRC_BOX_HPP_ */
