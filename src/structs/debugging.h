#ifndef DEBUGGING_H
#define DEBUGGING_H
#include <glm/glm.hpp>
#include <iostream>
#include <stdio.h>
void printvec3(glm::vec3 pos, const char *opt_string = "") {
  std::cout << opt_string << pos.x << ", " << pos.y << ", " << pos.z
            << std::endl;
}
#endif
