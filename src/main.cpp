#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <stdio.h>

// Create callback function for resizing window
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
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

  // Render loop
  while (!glfwWindowShouldClose(window)) {
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();

  return 0;
}
