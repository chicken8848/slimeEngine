#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <cmath>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <stdio.h>

#include "structs/Camera.h"
#include "structs/Shader.h"
#include "structs/stb_image.h"

#include "structs/Model.h"

#include <learnopengl/filesystem.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float mixValue = 0.2;

glm::mat4 view;

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f;

Camera ourCam = Camera();
bool first_mouse = true;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

// Create callback function for resizing window
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
  float xpos = static_cast<float>(xposIn);
  float ypos = static_cast<float>(yposIn);
  if (first_mouse) {
    lastX = xpos;
    lastY = ypos;
    first_mouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos;
  lastX = xpos;
  lastY = ypos;
  ourCam.ProcessMouseMovement(xoffset, yoffset, true);
}

void scroll_callback(GLFWwindow *window, double xpos, double ypos) {
  ourCam.ProcessMouseScroll(static_cast<float>(ypos));
}

// Processing Input
void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    mixValue += 0.01;
    if (mixValue > 1.0) {
      mixValue = 1.0;
    }
    printf("%f\n", mixValue);
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    mixValue -= 0.01;
    if (mixValue < 0) {
      mixValue = 0;
    }
    printf("%f\n", mixValue);
  }

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    ourCam.ProcessKeyboard(FORWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    ourCam.ProcessKeyboard(BACKWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    ourCam.ProcessKeyboard(LEFT, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    ourCam.ProcessKeyboard(RIGHT, deltaTime);
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

  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glm::vec3 pointLightPositions[] = {
      glm::vec3(0.7f, 0.2f, 2.0f), glm::vec3(2.3f, -3.3f, -4.0f),
      glm::vec3(-4.0f, 2.0f, -12.0f), glm::vec3(0.0f, 0.0f, -3.0f)};

  float vertices[] = {
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 0.0f,
      0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
      -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
      -0.5f, 0.5f,  0.5f,  0.0f, 1.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,

      -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f,
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  0.5f,  1.0f, 0.0f,

      0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
      0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 0.0f, 1.0f,
      0.5f,  -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 1.0f,
      0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

      -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f};

  unsigned int indices[] = {0, 1, 3, 1, 2, 3};

  glm::vec3 cubePositions[] = {
      glm::vec3(0.0f, 0.0f, 0.0f),    glm::vec3(2.0f, 5.0f, -15.0f),
      glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
      glm::vec3(2.4f, -0.4f, -3.5f),  glm::vec3(-1.7f, 3.0f, -7.5f),
      glm::vec3(1.3f, -2.0f, -2.5f),  glm::vec3(1.5f, 2.0f, -2.5f),
      glm::vec3(1.5f, 0.2f, -1.5f),   glm::vec3(-1.3f, 1.0f, -1.5f)};

  glEnable(GL_DEPTH_TEST);

  unsigned int VBO, VAO, EBO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);
  // Tell openGl how it should interpret the vertex data
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  stbi_set_flip_vertically_on_load(true);

  Shader ourShader("/home/chicken8848/Documents/programming/slimeEngine/src/"
                   "shaders/texture2.vert",
                   "/home/chicken8848/Documents/programming/slimeEngine/src/"
                   "shaders/texture2.frag");
  ourShader.use();

  ourShader.setVec3("pointLights[0].position", pointLightPositions[0]);
  ourShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
  ourShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
  ourShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
  ourShader.setFloat("pointLights[0].constant", 1.0f);
  ourShader.setFloat("pointLights[0].linear", 0.09f);
  ourShader.setFloat("pointLights[0].quadratic", 0.032f);

  Model testModel(FileSystem::getPath("assets/backpack/backpack.obj"));

  glEnable(GL_BLEND); // you enable blending function
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Render loop
  while (!glfwWindowShouldClose(window)) {

    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    // Inputs
    processInput(window);
    // Rendering Commands
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // use the program

    ourShader.use();

    glm::mat4 projection = glm::mat4(1.0f);
    projection =
        glm::perspective(glm::radians(ourCam.Zoom),
                         (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    ourShader.setMat4("projection", projection);
    view = ourCam.GetViewMatrix();
    ourShader.setMat4("view", view);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(
        model,
        glm::vec3(
            0.0f, 0.0f,
            0.0f)); // translate it down so it's at the center of the scene
    model = glm::scale(
        model,
        glm::vec3(1.0f, 1.0f,
                  1.0f)); // it's a bit too big for our scene, so scale it down
    ourShader.setMat4("model", model);
    testModel.Draw(ourShader);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
