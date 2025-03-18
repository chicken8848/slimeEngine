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

glm::vec3 gravity = glm::vec3(0.0f, -9.8f, 0.0f);



glm::mat4 view;

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f;

Camera ourCam = Camera(glm::vec3(0.0f, 0.0f, 5.0f));
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

  glm::vec3 velocity = glm::vec3(1.0f, 0.0f, 0.0f);


  glEnable(GL_DEPTH_TEST);

  //unsigned int VBO, VAO, EBO;
  //glGenVertexArrays(1, &VAO);
  //glBindVertexArray(VAO);
  //glGenBuffers(1, &VBO);
  //glGenBuffers(1, &EBO);
  //glBindBuffer(GL_ARRAY_BUFFER, VBO);
  //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
  //             GL_STATIC_DRAW);
  //// Tell openGl how it should interpret the vertex data
  //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  //glEnableVertexAttribArray(0);
  //glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
  //                      (void *)(3 * sizeof(float)));
  //glEnableVertexAttribArray(1);

  stbi_set_flip_vertically_on_load(true);

  //Shader ourShader("/home/chicken8848/Documents/programming/slimeEngine/src/"
  //                 "shaders/texture.vert",
  //                 "/home/chicken8848/Documents/programming/slimeEngine/src/"
  //                 "shaders/texture.frag");
  Shader ourShader("C:/Users/zq/Documents/GitHub/slimeEngine/src/shaders/texture.vert",
      "C:/Users/zq/Documents/GitHub/slimeEngine/src/shaders/texture.frag");

  //Model testModel(FileSystem::getPath("assets/backpack/backpack.obj"));
  //Model testModel(FileSystem::getPath("assets/pudding/pudding.obj"));
  Model testModel(FileSystem::getPath("assets/pudding/tetrapudding.obj"));

  glEnable(GL_BLEND); // you enable blending function
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

    velocity += gravity * deltaTime;
    
    glm::mat4 new_pos = glm::translate(model, velocity * deltaTime);
    if (new_pos[3][1] < -3) {
        velocity.y *= -1;
    }

    model = new_pos;

    ourShader.setMat4("model", model);
    testModel.Draw(ourShader);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
