#ifndef MESHLOADER_H
#define MESHLOADER_H

#include <vector>
#include <string>

// 3D Vector
struct Vector3 {
    double x, y, z;
};

// Tetrahedral element
struct Tetrahedron {
    int v1, v2, v3, v4;
};

class MeshLoader {
public:
    static std::vector<Vector3> loadNodes(const std::string& filename);
    static std::vector<Tetrahedron> loadTetrahedra(const std::string& filename);
};

#endif
