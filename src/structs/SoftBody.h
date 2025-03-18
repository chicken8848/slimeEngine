#ifndef SOFTBODY_H
#define SOFTBODY_H

#include "MeshLoader.h"

class SoftBody {
public:
    std::vector<Vector3> particles;
    std::vector<Tetrahedron> tetrahedra;

    void loadMesh(const std::string& nodeFile, const std::string& eleFile);
    void simulate();
};

#endif
