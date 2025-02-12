#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

class Shader {
public:
  unsigned int ID;

  Shader(const char *vertexPath, const char *fragmentPath) {
    std::string vertexCode;
    std::string fragmentCode;

    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
      vShaderFile.open(vertexPath);
      fShaderFile.open(fragmentPath);

      std::stringstream vShaderStream, fShaderStream;

      vShaderStream << vShaderFile.rdbuf();
      fShaderStream << fShaderFile.rdbuf();

      vShaderFile.close();
      vShaderFile.close();

      vertexCode = vShaderStream.str();
      fragmentCode = fShaderStream.str();

    } catch (std::ifstream::failure e) {
      std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ\n" << std::endl;
    }
    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();

    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompilerShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(vertex, 512, NULL, infoLog);
      std::cout << "ERROR_SHADER::VERTEX::COMPILATION_FAILED\n"
                << infoLog << std::endl;
    }
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vertex, 1, &fShaderCode, NULL);
    glCompilerShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(fragment, 512, NULL, infoLog);
      std::cout << "ERROR_SHADER::FRAGMENT::COMPILATION_FAILED\n"
                << infoLog << std::endl;
    }
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(ID, 512, NULL, infoLog);
      std::cout << "ERROR_SHADER::PROGRAM::LINKING_FAILED\n"
                << infoLog << std::endl;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
  };

  void use() { glUseProgram(ID); };

  void setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
  };
  void setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
  };
  void setFloat(const std::string &name, float value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
  };
};

#endif // !SHADER_H
