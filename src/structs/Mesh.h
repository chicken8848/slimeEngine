#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//#include "Hit.h"
//#include "Ray.h"
#include "Shader.h"

#include <string>
#include <unordered_map>
#include <vector>

#include <fstream>
#include <regex>
using namespace std;

bool stick_floor = false;

#define MAX_BONE_INFLUENCE 4

struct Vec3Hash {
    size_t operator()(const glm::ivec3& v) const {
        return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1) ^
            (std::hash<int>()(v.z) << 2);
    }
};

struct Particle {
    glm::vec3 pos;
    glm::vec3 prev_pos;
    glm::vec3 velocity;
    float mass;
    float inv_mass;
    Particle(glm::vec3 pos, float mass) {
        this->pos = pos;
        this->prev_pos = pos;
        this->velocity = { 0, 0, 0 };
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
    glm::vec2 particle_ids;
    float rest_length;
    Edge(float start_particle, float end_particle, float rest_length) {
        if (start_particle > end_particle)
            particle_ids = { end_particle, start_particle };
        else
            particle_ids = { start_particle, end_particle };
        this->rest_length = rest_length;
    }
    bool operator==(const Edge& other) const {
        return particle_ids == other.particle_ids;
    }

    bool operator<(const Edge& other) const {
        if (particle_ids.x == other.particle_ids.x) {
            return particle_ids.y < other.particle_ids.y;
        }
        else
            return particle_ids.x < other.particle_ids.x;
    }
};

struct Tetrahedron {
    glm::vec4 particle_ids;
    float rest_volume;
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
    vector<Particle> particle_reset;
    unordered_map<int, vector<int>> particle_vertex_map;
    vector<Tetrahedron> tetrahedrons;
    vector<Edge> edges;
    float edge_compliance;
    float volume_compliance;
    bool is_soft;

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices,
        vector<Texture> textures) {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        this->is_soft = false;

        setupMesh();
    }

    void initSoftBody(const string& node_path, const string& tetIDpath,
        float mass, float edge_compliance,
        float volume_compliance) {
        this->is_soft = true;
        this->edge_compliance = edge_compliance;
        this->volume_compliance = volume_compliance;
        // this->addParticles(node_path, mass);
        // this->addTetraIDs(tetIDpath);
        this->addParticlesTetGen(node_path, mass);
        this->addTetraIDsTetGen(tetIDpath);
        this->calcEdges();
    }

    void addParticles(const string& path, float mass) {
        fstream f(path);
        std::string line_buffer;

        regex reg("\\s+");

        while (getline(f, line_buffer)) {
            sregex_token_iterator iter(line_buffer.begin(), line_buffer.end(), reg,
                -1);
            sregex_token_iterator end;

            vector<string> vec(iter, end);

            float arg1 = std::stof(vec[0]);
            float arg2 = std::stof(vec[2]);
            float arg3 = std::stof(vec[1]);
            Particle p = Particle({ arg1, arg2, arg3 }, mass);
            this->particles.push_back(p);
        }
        f.close();
        this->particle_reset = this->particles;
        create_particle_vertex_map();
        return;
    }

    void addParticlesTetGen(const std::string& path, float mass) {
        std::ifstream file(path);
        std::string line;
        std::regex whitespace("\\s+");

        // Skip the first line: that has number of vertices etc
        std::getline(file, line);

        while (std::getline(file, line)) {
            // Remove leading/trailing spaces
            line = std::regex_replace(line, std::regex("^\\s+|\\s+$"), "");

            if (line.empty() || line[0] == '#')
                continue; // skip empty/comment lines

            std::sregex_token_iterator iter(line.begin(), line.end(), whitespace, -1);
            std::sregex_token_iterator end;
            std::vector<std::string> tokens(iter, end);

            if (tokens.size() < 4) {
                std::cerr << "Skipping short/malformed line: [" << line << "]\n";
                continue;
            }

            try {
                float x = std::stof(tokens[1]);
                float y = std::stof(tokens[2]); //for obj, 3, stl, 2
                float z = std::stof(tokens[3]);

                Particle p(glm::vec3(x, y, z), mass);
                this->particles.push_back(p);
            }
            catch (const std::exception& e) {
                std::cerr << "Invalid float data in line: [" << line << "]\n";
            }
        }
        file.close();
        this->particle_reset = this->particles;
        create_particle_vertex_map();
        return;
    }

    void create_particle_vertex_map() {
        const float epsilon = 0.1f;
        for (int i = 0; i < particles.size(); i++) {
            for (int j = 0; j < vertices.size(); j++) {
                if (glm::length(particles[i].pos - vertices[j].Position) <= epsilon) {
                    this->particle_vertex_map[i].push_back(j);
                }
            }
        }
    }

    void addTetraIDs(const string& path) {
        fstream f(path);
        std::string line_buffer;

        regex reg("\\s+");

        glm::vec4 tet;

        while (getline(f, line_buffer)) {
            sregex_token_iterator iter(line_buffer.begin(), line_buffer.end(), reg,
                -1);
            sregex_token_iterator end;

            vector<string> vec(iter, end);

            Tetrahedron tet;
            tet.particle_ids.x = std::stoi(vec[0]);
            tet.particle_ids.y = std::stoi(vec[1]);
            tet.particle_ids.z = std::stoi(vec[2]);
            tet.particle_ids.w = std::stoi(vec[3]);
            tet.rest_volume = getTetVolume(tet.particle_ids);

            particles[tet.particle_ids.x].inv_mass = 1 / (tet.rest_volume / 4);
            particles[tet.particle_ids.y].inv_mass = 1 / (tet.rest_volume / 4);
            particles[tet.particle_ids.z].inv_mass = 1 / (tet.rest_volume / 4);
            particles[tet.particle_ids.w].inv_mass = 1 / (tet.rest_volume / 4);

            particles[tet.particle_ids.x].mass =
                particles[tet.particle_ids.x].inv_mass;
            particles[tet.particle_ids.y].mass =
                particles[tet.particle_ids.y].inv_mass;
            particles[tet.particle_ids.z].mass =
                particles[tet.particle_ids.z].inv_mass;
            particles[tet.particle_ids.w].mass =
                particles[tet.particle_ids.w].inv_mass;

            this->tetrahedrons.push_back(tet);
        }
        f.close();
    }

    void addTetraIDsTetGen(const std::string& path) {
        std::ifstream file(path);
        std::string line;
        std::regex whitespace("\\s+");

        while (std::getline(file, line)) {
            // Trim leading/trailing spaces
            line = std::regex_replace(line, std::regex("^\\s+|\\s+$"), "");

            // Skip empty lines or comments
            if (line.empty() || line[0] == '#')
                continue;

            std::sregex_token_iterator iter(line.begin(), line.end(), whitespace, -1);
            std::sregex_token_iterator end;
            std::vector<std::string> tokens(iter, end);

            if (tokens.size() < 5) {
                continue;
            }

            try {
                Tetrahedron tet;
                // Adjusting for 1-based indices
                tet.particle_ids.x = std::stoi(tokens[1]) - 1;
                tet.particle_ids.y = std::stoi(tokens[2]) - 1;
                tet.particle_ids.z = std::stoi(tokens[3]) - 1;
                tet.particle_ids.w = std::stoi(tokens[4]) - 1;

                tet.rest_volume = getTetVolume(tet.particle_ids);

                particles[tet.particle_ids.x].inv_mass = 1 / (tet.rest_volume / 4);
                particles[tet.particle_ids.y].inv_mass = 1 / (tet.rest_volume / 4);
                particles[tet.particle_ids.z].inv_mass = 1 / (tet.rest_volume / 4);
                particles[tet.particle_ids.w].inv_mass = 1 / (tet.rest_volume / 4);

                particles[tet.particle_ids.x].mass =
                    particles[tet.particle_ids.x].inv_mass;
                particles[tet.particle_ids.y].mass =
                    particles[tet.particle_ids.y].inv_mass;
                particles[tet.particle_ids.z].mass =
                    particles[tet.particle_ids.z].inv_mass;
                particles[tet.particle_ids.w].mass =
                    particles[tet.particle_ids.w].inv_mass;

                tetrahedrons.push_back(tet);
            }
            catch (const std::exception& e) {
                std::cerr << "Exception parsing line: [" << line << "]\n"
                    << "Error: " << e.what() << "\n";
            }
        }

        file.close();
    }

    float getTetVolume(const glm::vec4& t) {
        Particle& point0 = particles[t.x];
        Particle& point1 = particles[t.y];
        Particle& point2 = particles[t.z];
        Particle& point3 = particles[t.w];

        glm::vec3 tempVec1 = point1.pos - point0.pos;
        glm::vec3 tempVec2 = point2.pos - point0.pos;
        glm::vec3 tempVec3 = point3.pos - point0.pos;

        float tetVolume = glm::dot(glm::cross(tempVec1, tempVec2), tempVec3) / 6.0f;

        return tetVolume;
    }

    void calcEdges() {
        for (auto tet : tetrahedrons) {
            glm::vec4 t = tet.particle_ids;
            Particle& point0 = particles[t.x];
            Particle& point1 = particles[t.y];
            Particle& point2 = particles[t.z];
            Particle& point3 = particles[t.w];
            Edge edge0 = Edge(t.x, t.y, glm::length(point0.pos - point1.pos));
            Edge edge1 = Edge(t.x, t.z, glm::length(point0.pos - point2.pos));
            Edge edge2 = Edge(t.x, t.w, glm::length(point0.pos - point3.pos));
            Edge edge3 = Edge(t.y, t.z, glm::length(point1.pos - point2.pos));
            Edge edge4 = Edge(t.y, t.w, glm::length(point1.pos - point3.pos));
            Edge edge5 = Edge(t.z, t.w, glm::length(point2.pos - point3.pos));
            this->edges.push_back(edge0);
            this->edges.push_back(edge1);
            this->edges.push_back(edge2);
            this->edges.push_back(edge3);
            this->edges.push_back(edge4);
            this->edges.push_back(edge5);
        }

        // Remove consecutive duplicates
        std::sort(this->edges.begin(), this->edges.end());
        auto last = std::unique(this->edges.begin(), this->edges.end());
        this->edges.erase(last, this->edges.end());
    }

    void pre_solve(float dt, glm::vec3 gravity) {
        for (int i = 0; i < particles.size(); i++) {
            if (particles[i].inv_mass == 0)
                continue;
            particles[i].velocity = particles[i].velocity + (gravity * dt);
            particles[i].prev_pos = particles[i].pos;
            particles[i].pos = particles[i].pos + particles[i].velocity * dt;
            if (particles[i].pos.y <= -2) {
                particles[i].pos = particles[i].prev_pos;
                particles[i].pos.y = -2;
            }
        }
    }

    void post_solve(float dt) {
        for (Particle& v : particles) {
            if (1.0 / dt >= INFINITY)
                continue;
            v.velocity =
                (v.pos - v.prev_pos) * 0.999f * (static_cast<float>(1.0 / dt));
            if (glm::length(v.velocity) <= 0.0002) {
                v.velocity = { 0, 0, 0 };
            }
        }
    }

    void solve_edges(float dt) {
        double alpha = this->edge_compliance / dt / dt;

        for (Edge& e : this->edges) {
            Particle& point0 = particles[e.particle_ids.x];
            Particle& point1 = particles[e.particle_ids.y];
            float w = point0.inv_mass + point1.inv_mass;
            if (w == 0)
                continue;
            glm::vec3 diffVec = point0.pos - point1.pos;
            float len = glm::length(diffVec);
            if (len == 0)
                continue;
            glm::vec3 normalizedVec = diffVec * (1.0f / len);
            float constraint_diff = len - e.rest_length;
            float l = -constraint_diff / (w + alpha);
            point0.pos = point0.pos + (normalizedVec * l * point0.inv_mass);
            point1.pos = point1.pos + (normalizedVec * -l * point1.inv_mass);
        }
    }

    void solve_volume(float dt) {
        double alpha = this->volume_compliance / dt / dt;

        std::vector<glm::vec3> volIdOrder = { glm::vec3(1, 3, 2), glm::vec3(0, 2, 3),
                                             glm::vec3(0, 3, 1),
                                             glm::vec3(0, 1, 2) };

        for (Tetrahedron& tet : this->tetrahedrons) {
            float w = 0;
            glm::vec3 diffVec[4];
            glm::vec3 gradients[4];
            for (int j = 0; j < 4; j++) {
                Particle& point0 = particles[tet.particle_ids[volIdOrder[j][0]]];
                Particle& point1 = particles[tet.particle_ids[volIdOrder[j][1]]];
                Particle& point2 = particles[tet.particle_ids[volIdOrder[j][2]]];

                glm::vec3 tempVec0 = point1.pos - point0.pos;
                glm::vec3 tempVec1 = point2.pos - point0.pos;
                diffVec[j] = glm::cross(tempVec0, tempVec1);
                gradients[j] = diffVec[j] * (1.0f / 6.0f);

                w += particles[tet.particle_ids[j]].inv_mass *
                    glm::dot(gradients[j], gradients[j]);
            }
            if (w == 0)
                continue;
            float volume = getTetVolume(tet.particle_ids);
            float constraint_diff = volume - tet.rest_volume;
            float l = -constraint_diff / (w + alpha);

            for (int j = 0; j < 4; j++) {
                Particle& p = particles[tet.particle_ids[j]];
                p.pos += gradients[j] * l * p.inv_mass;
            }
        }
    }

    void solve(float dt) {
        solve_edges(dt);
        solve_volume(dt);
    }

    void update(float dt, int substeps, glm::vec3 gravity) {
        float sdt = dt / substeps;
        for (int i = 0; i < substeps; i++) {
            pre_solve(sdt, gravity);
            solve(sdt);
            post_solve(sdt);
        }
        update_vertices();
    }

    void reset() { this->particles = this->particle_reset; }

    void Draw(Shader& shader) {
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
            }
            else if (name == "texture_specular") {
                number = std::to_string(specularNr++);
                shader.setInt("material.specular", i);
            }
            else if (name == "texture_normal") {
                number = std::to_string(normalNr++);
                shader.setInt("material.normal", i);
            }
            else if (name == "texture_height") {
                number = std::to_string(heightNr++);
                shader.setInt("material.height", i);
            }
            shader.setFloat("material.shininess", 32.0f);

            glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()),
            GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
    }

    /// Ray-triangle intersection test
   /* bool triangle_intersect(const Ray& ray, Hit& hit, float tmin, glm::vec3 a,
        glm::vec3 b, glm::vec3 c) {
        vec3 origin = ray.getOrigin();
        vec3 dir = ray.getDirection();

        mat3 A = mat3(a[0] - b[0], a[0] - c[0], dir[0], a[1] - b[1], a[1] - c[1],
            dir[1], a[2] - b[2], a[2] - c[2], dir[2]);

        mat3 betaMat =
            mat3(a[0] - origin[0], a[0] - c[0], dir[0], a[1] - origin[1],
                a[1] - c[1], dir[1], a[2] - origin[2], a[2] - c[2], dir[2]);

        mat3 gammaMat =
            mat3(a[0] - b[0], a[0] - origin[0], dir[0], a[1] - b[1],
                a[1] - origin[1], dir[1], a[2] - b[2], a[2] - origin[2], dir[2]);

        mat3 tMat = mat3(a[0] - b[0], a[0] - c[0], a[0] - origin[0], a[1] - b[1],
            a[1] - c[1], a[1] - origin[1], a[2] - b[2], a[2] - c[2],
            a[2] - origin[2]);

        float gamma = determinant(gammaMat) / determinant(A);
        float beta = determinant(betaMat) / determinant(A);
        float alpha = 1.0f - beta - gamma;
        float t = determinant(tMat) / determinant(A);

        if (beta + gamma > 1 || beta < 0 || gamma < 0) {
            return false;
        }

        if (t > tmin && t < hit.getT()) {
            hit.set(t);

            return true;
        }
        else {
            return false;
        }
    }

    bool intersect(const Ray& r, Hit& h, float tmin) {
        bool result = false;
        for (unsigned int i = 0; i < indices.size() / 3; i++) {
            glm::vec3 a = vertices[this->indices[3 * i]].Position;
            glm::vec3 b = vertices[this->indices[3 * i + 1]].Position;
            glm::vec3 c = vertices[this->indices[3 * i + 2]].Position;
            result |= triangle_intersect(r, h, tmin, a, b, c);
        }
        return result;
    }*/

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

    void update_vertices() {
        for (int i = 0; i < particles.size(); i++) {
            for (auto& j : particle_vertex_map[i]) {
                vertices[j].Position = particles[i].pos;
            }
        }
        setupMesh();
    }
};

#endif