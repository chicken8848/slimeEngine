#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"
#include <string>
#include <vector>
using namespace std;

#define MAX_BONE_INFLUENCE 4

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    float m_Weights[MAX_BONE_INFLUENCE];
    glm::vec3 Velocity; // Added for physics simulation
};

struct Texture {
    unsigned int id;
    string type;
    string path;
};

// Define a constraint struct
struct DistanceConstraint {
    int v1, v2; // Indices of the two vertices involved in the constraint
    float restLength; // The desired distance between the vertices
};

struct VolumeConstraint {
    int tetId;         // Index of the tetrahedron in tetIds
    float restVolume;  // Rest volume of the tetrahedron
};
class Mesh {
public:
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    std::vector<glm::vec4> tetIds; // Tetrahedra vertex indices
    std::vector<DistanceConstraint> distanceConstraints; // Volume constraints
    std::vector<VolumeConstraint> volumeConstraints; // Volume constraints

    // Constructor
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices,
        vector<Texture> textures, std::vector<glm::vec4> tetIds)
        : vertices(vertices), indices(indices), textures(textures), tetIds(tetIds) {
        setupMesh();
        setupDistanceConstraints(); // Initialize distance constraints
        setupVolumeConstraints(); // Initialize volume constraints
    }

    // Function to initialize distance constraints
    void setupDistanceConstraints() {
        for (const auto& tet : tetIds) {
            // Get the four vertices of the tetrahedron
            int v0 = tet.x;
            int v1 = tet.y;
            int v2 = tet.z;
            int v3 = tet.w;

            // Add constraints for all edges of the tetrahedron
            addDistanceConstraint(v0, v1);
            addDistanceConstraint(v0, v2);
            addDistanceConstraint(v0, v3);
            addDistanceConstraint(v1, v2);
            addDistanceConstraint(v1, v3);
            addDistanceConstraint(v2, v3);
        }
    }

    // Function to add a distance constraint for a tetrahedron
    void addDistanceConstraint(int v1, int v2) {
        // Calculate the rest length (distance between the two vertices)
        glm::vec3 p1 = vertices[v1].Position;
        glm::vec3 p2 = vertices[v2].Position;
        float restLength = glm::length(p2 - p1);

        // Add the constraint to the list
        distanceConstraints.push_back({ v1, v2, restLength });
    }


    // Function to initialize volume constraints
    void setupVolumeConstraints() {
        for (size_t i = 0; i < tetIds.size(); i++) {
            addVolumeConstraint(i);
        }
    }

    // Function to add a volume constraint for a tetrahedron
    void addVolumeConstraint(int tetId) {
        glm::vec4 tet = tetIds[tetId];
        Vertex& v1 = vertices[tet.x];
        Vertex& v2 = vertices[tet.y];
        Vertex& v3 = vertices[tet.z];
        Vertex& v4 = vertices[tet.w];

        glm::vec3 edge1 = v2.Position - v1.Position;
        glm::vec3 edge2 = v3.Position - v1.Position;
        glm::vec3 edge3 = v4.Position - v1.Position;

        // Calculate the signed volume
        float signedVolume = glm::dot(edge1, glm::cross(edge2, edge3)) / 6.0f;

        // Use the absolute value for the rest volume
        float restVolume = std::abs(signedVolume);
        //float restVolume = signedVolume;
        //float restVolume = 10.0f;

        // Print the rest volume for debugging
        //std::cout << "Rest volume for tetrahedron " << tetId << ": " << restVolume << std::endl;

        // Add the volume constraint
        volumeConstraints.push_back({ tetId, restVolume });
    }

    // Function to solve volume constraints
//    void solveVolumeConstraints() {
// 
//};

    void Draw(Shader& shader) {
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;
        for (unsigned int i = 0; i < textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            string number;
            string name = textures[i].type;
            if (name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if (name == "texture_specular")
                number = std::to_string(specularNr++);
            else if (name == "texture_normal")
                number = std::to_string(normalNr++);
            else if (name == "texture_height")
                number = std::to_string(heightNr++);

            glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()),
            GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
    };

//private:
    unsigned int VAO;
    unsigned int VBO, EBO;

    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
            &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
            &indices[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            (void*)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            (void*)offsetof(Vertex, TexCoords));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            (void*)offsetof(Vertex, Tangent));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            (void*)offsetof(Vertex, Bitangent));
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex),
            (void*)offsetof(Vertex, m_BoneIDs));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            (void*)offsetof(Vertex, m_Weights));
        glBindVertexArray(0);
    }
};

#endif