#ifndef RAY_H
#define RAY_H

#include <cassert>
#include <iostream>
// #include "vec3.h"
#include <glm/glm.hpp>
using namespace std;
using namespace glm;

class Ray {
public:
  Ray(const vec3 &orig, const vec3 &dir) {
    origin = orig;
    direction = dir;
  }

  Ray(const Ray &r) {
    origin = r.origin;
    direction = r.direction;
  }

  const vec3 &getOrigin() const { return origin; }

  const vec3 &getDirection() const { return direction; }

  vec3 pointAtParameter(float t) const { return origin + direction * t; }

private:
  // don't use this constructor
  Ray() { assert(false); }

  vec3 origin;
  vec3 direction;
};

inline ostream &operator<<(ostream &os, const Ray &r) {
  float origin[3];
  origin[0] = r.getOrigin()[0];
  origin[1] = r.getOrigin()[1];
  origin[2] = r.getOrigin()[2];

  float direction[3];
  direction[0] = r.getDirection()[0];
  direction[1] = r.getDirection()[1];
  direction[2] = r.getDirection()[2];
  os << "Ray <" << origin << ", " << direction << ">";
  return os;
}

#endif // RAY_H
