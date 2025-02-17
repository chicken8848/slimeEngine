#ifndef MESH_H
#define MESH_H

#include <cstddef>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"

#include <string>
#include <vector>

#define MAX_BONE_INFLUENCE 4

struct Texture {
  unsigned int id;
  std::string type;
  std::string path;
};

struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
  glm::vec3 Tangent;
  glm::vec3 Bittangent;
  int m_BoneIDs[MAX_BONE_INFLUENCE];
  float m_Weights[MAX_BONE_INFLUENCE];
};

class Mesh {
public:
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;
  Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
       std::vector<Texture> textures) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    setupMesh();
  }
  void Draw(Shader &shader) {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;

    for (unsigned int i = 0; i < textures.size(); i++) {
      glActiveTexture(GL_TEXTURE0 + i);
      std::string number;
      std::string name = textures[i].type;
      if (name == "texture_diffuse") {
        number = std::to_string(diffuseNr++);
      } else if (name == "texture_specular") {
        number = std::to_string(specularNr++);
      } else if (name == "texture_normal") {
        number = std::to_string(normalNr++);
      } else if (name == "texture_height") {
        number = std::to_string(heightNr++);
      }
      glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
      glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
  }

private:
  unsigned int VAO, VBO, EBO;

  void setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffer(1, &EBO);
    glGenBuffer(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBindBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
                     &vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, EBO);
    glBindBufferData(GL_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                     &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, Tangent));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, Bittangent));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_INT, sizeof(Vertex),
                          (void *)offsetof(Vertex, m_BoneIDs));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, m_Weights));

    glBindVertexArray(0);
  }
};

#endif // !MESH_H
