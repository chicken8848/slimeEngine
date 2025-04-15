#ifndef HIT_H
#define HIT_H

#include "Ray.h"
#include <float.h>
#include <glm/glm.hpp>

class Material;

class Hit {
public:
  // constructors
  Hit() {
    material = NULL;
    t = FLT_MAX;
    hasTex = false;
  }

  Hit(float _t, Material *m, const vec3 &n) {
    t = _t;
    material = m;
    normal = n;
    hasTex = false;
  }

  Hit(const Hit &h) {
    t = h.t;
    material = h.material;
    normal = h.normal;
    hasTex = h.hasTex;
  }

  // destructor
  ~Hit() {}

  float getT() const { return t; }

  Material *getMaterial() const { return material; }

  const vec3 &getNormal() const { return normal; }

  void set(float _t) { t = _t; }

  void setTexCoord(const vec2 &coord) {
    texCoord = coord;
    hasTex = true;
  }

  bool hasTex;
  vec2 texCoord;
  float alpha;
  float gamma;
  float beta;

private:
  float t;
  Material *material;
  vec3 normal;
};

inline ostream &operator<<(ostream &os, const Hit &h) {
  float normal[3];
  normal[0] = h.getNormal()[0];
  normal[1] = h.getNormal()[1];
  normal[2] = h.getNormal()[2];
  os << "Hit <" << h.getT() << ", " << normal << ">";
  return os;
}

#endif // HIT_H
