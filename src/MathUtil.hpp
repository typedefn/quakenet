/*
 * MathUtil.hpp
 *
 *  Created on: Jun 22, 2020
 *      Author: dervish
 */

#ifndef SRC_MATHUTIL_HPP_
#define SRC_MATHUTIL_HPP_
#include <Common.hpp>
#include <Ray.hpp>
#include <Utility.hpp>

#define lerpFloat(a0, a1, w) ((a0) + (w)*((a1) - (a0)))
#define DEG_TO_RAD(ang) ((ang)*PI/180.0)
#define RAD_TO_DEG(rads) ((rads)*180.0/PI)

enum Axis {
  X,
  Y,
  Z
};

class MathUtil {
	MathUtil();
	virtual ~MathUtil();

public:
	static MathUtil* instance();

	bool intersect(const Ray &r, const glm::vec3 &p, float halfWidth);
	bool aabbIntersect(const glm::vec3 &p1, const glm::vec3 &p2, float w);
	int randomRanged(int a, int b);
	float randomUnit();
	float randomUnit(int x, int z, int seed);
	float randomUnit(int x, int y, int z, int seed);
	float linearInterpolate(float a, float b, float x);
	float grad(int hash, float x, float y, float z);
	void findMin(glm::vec3 point, glm::vec3 &min);
	void findMax(glm::vec3 point, glm::vec3 &max);
};

#endif /* SRC_MATHUTIL_HPP_ */
