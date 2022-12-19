/*
 * Box.cpp
 *
 *  Created on: Oct 16, 2020
 *      Author: dervish
 */

#include <Box.hpp>

Box::Box() {
  bounds[0] = glm::vec3(0);
  bounds[1] = glm::vec3(0);
  extra = 0;
}

Box::Box(const glm::vec3 &min, const glm::vec3 &max) {
  bounds[0] = min;
  bounds[1] = max;
  extra = 0;
}

Box::~Box() {
  // TODO Auto-generated destructor stub
}
bool Box::contains(const Box & b) const {
  glm::vec3 bmin = b.getMin();
  glm::vec3 bmax = b.getMax();

  glm::vec3 min = bounds[0];
  glm::vec3 max = bounds[1];
 
/*  LOG << min.x << " " << bounds[0].x << " " << min.z << " " << bounds[0].z 
      << min.y << " " << bounds[0].y << " " << max.x << " " << bounds[1].x
      << max.y << " " << bounds[1].y << " " << max.z << " " << bounds[1].z;
*/
return (
    min.x <= bmax.x &&
    max.x >= bmin.x &&
    min.y <= bmax.y &&
    max.y >= bmin.y &&
    min.z <= bmax.z &&
    max.z >= bmin.z
  );
  //return (min.x >= bounds[0].x && min.y >= bounds[0].y && min.z >= bounds[0].z && max.x <= bounds[1].x && max.y <= bounds[1].y && max.z <= bounds[1].z);
}

bool Box::intersect(const Ray &r) const {
  glm::vec3 min = bounds[0];
  glm::vec3 max = bounds[1];

  float tmin = (min.x - r.origin().x) / r.direction().x; 
  float tmax = (max.x - r.origin().x) / r.direction().x; 
 
  if (tmin > tmax) std::swap(tmin, tmax); 

  float tymin = (min.y - r.origin().y) / r.direction().y; 
  float tymax = (max.y - r.origin().y) / r.direction().y; 

  if (tymin > tymax) std::swap(tymin, tymax); 

  if ((tmin > tymax) || (tymin > tmax)) 
      return false; 

  if (tymin > tmin) 
      tmin = tymin; 

  if (tymax < tmax) 
      tmax = tymax; 

  float tzmin = (min.z - r.origin().z) / r.direction().z; 
  float tzmax = (max.z - r.origin().z) / r.direction().z; 

  if (tzmin > tzmax) std::swap(tzmin, tzmax); 

  if ((tmin > tzmax) || (tzmin > tmax)) 
      return false; 

  if (tzmin > tmin) 
      tmin = tzmin; 

  if (tzmax < tmax) 
      tmax = tzmax; 

  return true; 
}

bool Box::intersect(const Ray &r, float t0, float t1) const {

  float tmin, tmax, tymin, tymax, tzmin, tzmax;
  float divx = 1 / r.direction().x;
  float divy = 1 / r.direction().y; 
  float divz = 1 / r.direction().z;

  if (divx >= 0) {
    tmin = (bounds[0].x - r.origin().x) * divx;
    tmax = (bounds[1].x - r.origin().x) * divx;
  } else {
    tmin = (bounds[1].x - r.origin().x) * divx;
    tmax = (bounds[0].x - r.origin().x) * divx;
  }

  if (divy >= 0) {
    tymin = (bounds[0].y - r.origin().y) / r.direction().y;
    tymax = (bounds[1].y - r.origin().y) / r.direction().y;
  } else {
    tymin = (bounds[1].y - r.origin().y) / r.direction().y;
    tymax = (bounds[0].y - r.origin().y) / r.direction().y;
  }
  if ((tmin > tymax) || (tymin > tmax)) {
    return false;
  }

  if (tymin > tmin) {
    tmin = tymin;
  }
  if (tymax < tmax) {
    tmax = tymax;
  }
  if (divz >= 0) {
    tzmin = (bounds[0].z - r.origin().z) / r.direction().z;
    tzmax = (bounds[1].z - r.origin().z) / r.direction().z;
  } else {
    tzmin = (bounds[1].z - r.origin().z) / r.direction().z;
    tzmax = (bounds[0].z - r.origin().z) / r.direction().z;
  }

  if ((tmin > tzmax) || (tzmin > tmax)) {
    return false;
  }
  if (tzmin > tmin) {
    tmin = tzmin;
  }
  if (tzmax < tmax) {
    tmax = tzmax;
  }
  return ((tmin < t1) && (tmax > t0));
}


bool Box::intersectV2(const Ray &r, float *t) {
  // r.dir is unit direction vector of ray
  glm::vec3 dirfrac;
  dirfrac.x = 1.0f / r.direction().x;
  dirfrac.y = 1.0f / r.direction().y;
  dirfrac.z = 1.0f / r.direction().z;
  // lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
  // r.org is origin of ray
  glm::vec3 lb = bounds[0];
  glm::vec3 rt = bounds[1];
  float t1 = (lb.x - r.origin().x)*dirfrac.x;
  float t2 = (rt.x - r.origin().x)*dirfrac.x;
  float t3 = (lb.y - r.origin().y)*dirfrac.y;
  float t4 = (rt.y - r.origin().y)*dirfrac.y;
  float t5 = (lb.z - r.origin().z)*dirfrac.z;
  float t6 = (rt.z - r.origin().z)*dirfrac.z;

  float tmin = glm::max(glm::max(glm::min(t1, t2), glm::min(t3, t4)), glm::min(t5, t6));
  float tmax = glm::min(glm::min(glm::max(t1, t2), glm::max(t3, t4)), glm::max(t5, t6));

  // if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
  if (tmax < 0)
  {
      *t = tmax;
      return false;
  }

  // if tmin > tmax, ray doesn't intersect AABB
  if (tmin > tmax)
  {
      *t = tmax;
      return false;
  }

  *t = tmin;
  return true;
}
