/*
 * MathUtil.cpp
 *
 *  Created on: Jun 22, 2020
 *      Author: dervish
 */

#include "MathUtil.hpp"

MathUtil::MathUtil() {
  // TODO Auto-generated constructor stub

}

MathUtil::~MathUtil() {
  // TODO Auto-generated destructor stub
}

MathUtil* MathUtil::instance() {
  static MathUtil instance;
  return &instance;
}

bool MathUtil::intersect(const Ray &r, const glm::vec3 &p, float halfWidth) {
  glm::vec3 min = glm::vec3(p.x - halfWidth, p.y - halfWidth, p.z - halfWidth);
  glm::vec3 max = glm::vec3(p.x + halfWidth, p.y + halfWidth, p.z + halfWidth);
  float tmin = (min.x - r.origin().x) / r.direction().x;
  float tmax = (max.x - r.origin().x) / r.direction().x;

  if (tmin > tmax)
    Utility::swap(tmin, tmax);

  float tymin = (min.y - r.origin().y) / r.direction().y;
  float tymax = (max.y - r.origin().y) / r.direction().y;

  if (tymin > tymax)
    Utility::swap(tymin, tymax);

  if ((tmin > tymax) || (tymin > tmax))
    return false;

  if (tymin > tmin)
    tmin = tymin;

  if (tymax < tmax)
    tmax = tymax;

  float tzmin = (min.z - r.origin().z) / r.direction().z;
  float tzmax = (max.z - r.origin().z) / r.direction().z;

  if (tzmin > tzmax)
    Utility::swap(tzmin, tzmax);

  if ((tmin > tzmax) || (tzmin > tmax))
    return false;

  if (tzmin > tmin)
    tmin = tzmin;

  if (tzmax < tmax)
    tmax = tzmax;

  return true;
}

bool MathUtil::aabbIntersect(const glm::vec3 &p1, const glm::vec3 &p2, float w) {
  float combinedWidth = w * 2;

  bool x = std::fabs(p1.x - p2.x) <= (combinedWidth);
  bool y = std::fabs(p1.y - p2.y) <= (combinedWidth);
  bool z = std::fabs(p1.z - p2.z) <= (combinedWidth);

  return x && y && z;
}

int MathUtil::randomRanged(int a, int b) {
  return (a + (std::rand() % (b - a + 1)));
}

float MathUtil::randomUnit() {
  int r = randomRanged(0, 100);
  return r / 100.0f;
}

float MathUtil::randomUnit(int x, int z, int seed) {
  int n = x + z * 57 + seed * 121;
  n = (n << 13) ^ n;
  return (1.0 - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}

float MathUtil::randomUnit(int x, int y, int z, int seed) {
  int n = x + y + z * 57 + seed * 121;
  n = (n << 13) ^ n;
  return (1.0 - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}

float MathUtil::linearInterpolate(float a, float b, float x) {
  return a * (1 - x) + b * x;
}

void MathUtil::findMin(glm::vec3 point, glm::vec3 &min) {
  min.x = glm::min(point.x, min.x);
  min.y = glm::min(point.y, min.y);
  min.z = glm::min(point.z, min.z);
}

void MathUtil::findMax(glm::vec3 point, glm::vec3 &max) {
  max.x = glm::max(point.x, max.x);
  max.y = glm::max(point.y, max.y);
  max.z = glm::max(point.z, max.z);
}

