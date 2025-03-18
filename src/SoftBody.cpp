#include "SoftBody.h"
#include <iostream>

void SoftBody::loadMesh(const std::string& nodeFile, const std::string& eleFile) {
    particles = MeshLoader::loadNodes(nodeFile);
    tetrahedra = MeshLoader::loadTetrahedra(eleFile);

    std::cout << "Loaded " << particles.size() << " particles and "
        << tetrahedra.size() << " tetrahedra." << std::endl;
}

void SoftBody::simulate() {
    // XPBD simulation loop here
    std::cout << "Running soft body simulation..." << std::endl;
}
