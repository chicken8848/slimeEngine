#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>

#include "Mesh.h"
#include "Shader.h"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);

class Model {
public:
    vector<Texture> textures_loaded; // Stores all textures loaded so far
    vector<Mesh> meshes;            // Stores all meshes in the model
    string directory;               // Directory of the model file
    bool gammaCorrection;           // Whether to apply gamma correction

    // Constructor
    Model(string const& path, bool gamma = false) : gammaCorrection(gamma) {
        loadModel(path);
    }

    // Draw the model
    void Draw(Shader& shader) {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

private:
    // Load the model from file
    void loadModel(string const& path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(
            path, aiProcess_GenSmoothNormals |
            aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices);
        //aiProcess_Triangulate
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        directory = path.substr(0, path.find_last_of('/')); // Extract directory
        processNode(scene->mRootNode, scene); // Process the root node
    }

    // Process a node in the scene
    void processNode(aiNode* node, const aiScene* scene) {
        // Process all meshes in the node
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // Process all child nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

    std::vector<glm::vec3> loadNodes(const std::string& filename) {
        std::ifstream file(filename);
        std::vector<glm::vec3> nodes;

        if (!file) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return nodes;
        }

        int numNodes, dim, attr, boundary;
        file >> numNodes >> dim >> attr >> boundary; // Read header

        for (int i = 0; i < numNodes; ++i) {
            int index;
            glm::vec3 v;
            file >> index >> v.x >> v.y >> v.z;
            nodes.push_back(v);
        }

        return nodes;
    }

    // Load tetrahedra data from a file
    std::vector<glm::vec4> loadTetrahedra(const std::string& filename) {
        std::ifstream file(filename);
        std::vector<glm::vec4> tets;

        if (!file) {
            std::cerr << "Error: Cannot open file " << filename << std::endl;
            return tets;
        }

        int numTets, nodesPerTet, attr;
        file >> numTets >> nodesPerTet >> attr;

        for (int i = 0; i < numTets; ++i) {
            int index;
            glm::vec4 t;
            file >> index >> t.x >> t.y >> t.z >> t.w;
            tets.push_back(t);
        }

        return tets;
    }

    // Load tetrahedra data from a file (generated from blender)
    std::vector<glm::vec4> loadTetrahedra2(const std::string& filename) {
        std::ifstream file(filename);
        std::vector<glm::vec4> tets;

        if (!file) {
            std::cerr << "Error: Cannot open file " << filename << std::endl;
            return tets;
        }

        // Read until the end of the file
        while (file) {
            //int index;
            glm::vec4 t;

            // Attempt to read the data
            file >> t.x >> t.y >> t.z >> t.w;

            // If the read was successful, add the tetrahedron to the vector
            if (file) {
                tets.push_back(t);
            }
        }

        return tets;
    }

    // Process a mesh and convert it to our Mesh class
    Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        // Load tetrahedra data
        std::vector<glm::vec4> tetIds = loadTetrahedra2("C:/Users/zq/Documents/GitHub/slimeEngine/assets/pudding/pudding.ele");
        //std::vector<glm::vec4> tetIds = loadTetrahedra("C:/Users/zq/Downloads/tetgen1.6.0/tetgen1.6.0/build/Debug/puddings/pudding.1.ele");

        //vector<glm::vec3> verticespos = loadNodes("C:/Users/zq/Downloads/tetgen1.6.0/tetgen1.6.0/build/Debug/puddings/pudding.1.node");
        // Process vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        //for (unsigned int i = 0; i < verticespos.size(); i++) {
        //for (unsigned int i = 0; i < 72; i++) {
            Vertex vertex;
            glm::vec3 vector;

            // Position
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;

            vertex.Position = vector;
            //vertex.Position = verticespos[i];

            // Normal
            if (mesh->HasNormals()) {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }

            // Texture coordinates
            if (mesh->mTextureCoords[0]) {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;

                // Tangent
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;

                // Bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
            else {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            // Initialize velocity for physics simulation
            vertex.Velocity = glm::vec3(0.0f);

            //vertex.inverseMass = 1.0f;

            vertices.push_back(vertex);
        }

        // Process indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // Process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        // Return a Mesh object
        return Mesh(vertices, indices, textures, tetIds);
    }

    // Load textures from a material
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName) {
        vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str);
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++) {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }
            if (!skip) {
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);
            }
        }
        return textures;
    }
};

// Utility function to load a texture from file
unsigned int TextureFromFile(const char* path, const string& directory, bool gamma) {
    string filename = string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

#endif