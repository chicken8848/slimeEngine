#include "MeshLoader.h"
#include <fstream>
#include <iostream>
#include <sstream>

std::vector<Vector3> MeshLoader::loadNodes(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<Vector3> nodes;

    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return nodes;
    }

    int numNodes, dim, attr, boundary;
    file >> numNodes >> dim >> attr >> boundary;

    for (int i = 0; i < numNodes; ++i) {
        int index;
        Vector3 v;
        file >> index >> v.x >> v.y >> v.z;
        nodes.push_back(v);
    }

    return nodes;
}

std::vector<Tetrahedron> MeshLoader::loadTetrahedra(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<Tetrahedron> tets;

    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return tets;
    }

    int numTets, nodesPerTet, attr;
    file >> numTets >> nodesPerTet >> attr;

    for (int i = 0; i < numTets; ++i) {
        int index;
        Tetrahedron t;
        file >> index >> t.v1 >> t.v2 >> t.v3 >> t.v4;
        tets.push_back(t);
    }

    return tets;
}
