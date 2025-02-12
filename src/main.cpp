#include <GL/gl.h>
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <stdio.h>

#include "shader.h"

// Create callback function for resizing window
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

// Processing Input
void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
  }
}
int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create a window object
  GLFWwindow *window = glfwCreateWindow(800, 600, "slimeEngine", NULL, NULL);

  if (window == NULL) {
    printf("Failed to create GLFW Window");
    glfwTerminate();
    return -1;
  }

  // Make main thread context the window context
  glfwMakeContextCurrent(window);

  // init GLAD so we can manage function pointers to OpenGL
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    printf("Failed to initialize GLAD");
    glfwTerminate();
    return -1;
  }

  // Tell openGl the size of rendering window
  glViewport(0, 0, 800, 600);

  // tell GLFW to call this function on every window resize
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  float vertices[] = {
      // positions         // colors
      0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom right
      -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom left
      0.0f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f  // top
  };

  float texCoords[] = {0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 1.0f};

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

  float borderColor[] = {1.0f, 1.0f, 0.0f, 1.0f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

  unsigned int indices[] = {0, 1, 3, 1, 2, 3};

  unsigned int VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  // Tell openGl how it should interpret the vertex data
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  Shader ourShader("/home/chicken8848/Documents/programming/slimeEngine/src/"
                   "shaders/pos_as_color.vert",
                   "/home/chicken8848/Documents/programming/slimeEngine/src/"
                   "shaders/waves.frag");

  // Render loop
  while (!glfwWindowShouldClose(window)) {
    // Inputs
    processInput(window);
    // Rendering Commands
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // use the program
    ourShader.use();

    glBindVertexArray(VAO);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);

  glfwTerminate();

  return 0;
}
