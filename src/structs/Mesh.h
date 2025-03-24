#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"

#include <string>
#include <unordered_map>
#include <vector>

#include <fstream>
#include <regex>
using namespace std;

#define MAX_BONE_INFLUENCE 4

struct Particle {
  glm::vec3 pos;
  glm::vec3 prev_pos;
  glm::vec3 velocity;
  float mass;
  float inv_mass;
  Particle(glm::vec3 pos, float mass) {
    this->pos = pos;
    this->prev_pos = pos;
    this->velocity = {0, 0, 0};
    this->mass = mass;
    if (this->mass != 0)
      this->inv_mass = 1 / mass;
    else
      this->inv_mass = 0;
  }
};

struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
  glm::vec3 Tangent;
  glm::vec3 Bitangent;
  int m_BoneIDs[MAX_BONE_INFLUENCE];
  float m_Weights[MAX_BONE_INFLUENCE];
};

struct Edge {
  glm::vec2 edge_ids;
  float rest_length;
  float edge_length;
  Edge(float start_particle, float end_particle, float rest_length) {
    this->edge_ids = {start_particle, end_particle};
    this->rest_length = rest_length;
  }
};

struct Texture {
  unsigned int id;
  string type;
  string path;
};

class Mesh {
public:
  vector<Vertex> vertices;
  vector<unsigned int> indices;
  vector<Texture> textures;
  unsigned int VAO;

  // soft body attributes
  vector<Particle> particles;
  unordered_map<int, vector<int>> particle_vertex_map;
  vector<glm::vec4> tetraIds;
  vector<Edge> edges;

  Mesh(vector<Vertex> vertices, vector<unsigned int> indices,
       vector<Texture> textures) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    setupMesh();
    std::cout << "Mesh successfully loaded" << std::endl;
    std::cout << "Vertices size: " << this->vertices.size() << std::endl;
    std::cout << "Textures size: " << this->textures.size() << std::endl;
    std::cout << "Indices size: " << this->indices.size() << std::endl;
  }

  void addParticles(const string &path, float mass) {
    fstream f(path);
    std::string line_buffer;

    regex reg("\\s+");

    while (getline(f, line_buffer)) {
      sregex_token_iterator iter(line_buffer.begin(), line_buffer.end(), reg,
                                 -1);
      sregex_token_iterator end;

      vector<string> vec(iter, end);

      float arg1 = std::stof(vec[0]);
      float arg2 = std::stof(vec[1]);
      float arg3 = std::stof(vec[2]);
      Particle p = Particle({arg1, arg2, arg3}, mass);
      this->particles.push_back(p);
    }
    f.close();

    create_particle_vertex_map();
    return;
  }

  void create_particle_vertex_map() {
    const float epsilon = 0.001;
    for (int i = 0; i < particles.size(); i++) {
      for (int j = 0; j < vertices.size(); j++) {
        if (glm::length(particles[i].pos - vertices[j].Position) <= epsilon) {
          this->particle_vertex_map[i].push_back(j);
        }
      }
    }
  }

  void addTetraIDs(const string &path) {
    fstream f(path);
    std::string line_buffer;

    regex reg("\\s+");

    glm::vec4 tet;

    while (getline(f, line_buffer)) {
      sregex_token_iterator iter(line_buffer.begin(), line_buffer.end(), reg,
                                 -1);
      sregex_token_iterator end;

      vector<string> vec(iter, end);

      tet.x = std::stof(vec[0]);
      tet.y = std::stof(vec[1]);
      tet.z = std::stof(vec[2]);
      tet.w = std::stof(vec[3]);
      this->tetraIds.push_back(tet);
    }
    f.close();
  }
  void calcEdges();

  void Draw(Shader &shader) {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;
    for (unsigned int i = 0; i < textures.size(); i++) {
      glActiveTexture(GL_TEXTURE0 + i);
      string number;
      string name = textures[i].type;
      if (name == "texture_diffuse") {
        number = std::to_string(diffuseNr++);
        shader.setInt("material.diffuse", i);
      } else if (name == "texture_specular") {
        number = std::to_string(specularNr++);
        shader.setInt("material.specular", i);
      } else if (name == "texture_normal") {
        number = std::to_string(normalNr++);
        shader.setInt("material.normal", i);
      } else if (name == "texture_height") {
        number = std::to_string(heightNr++);
        shader.setInt("material.height", i);
      }
      shader.setFloat("material.shininess", 64.0f);

      glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
      glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()),
                   GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
  }

private:
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, Tangent));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, Bitangent));
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex),
                           (void *)offsetof(Vertex, m_BoneIDs));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, m_Weights));
    glBindVertexArray(0);
  }
};

#endif
