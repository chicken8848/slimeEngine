#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

#include "structs/Camera.h"
#include "structs/Shader.h"
#include "structs/Model.h"
#include "structs/stb_image.h"
#include <learnopengl/filesystem.h>

// Window dimensions
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Global variables
float deltaTime = 0.0f; // Time between frames
float lastFrame = 0.0f; // Time of the last frame

// Camera
Camera ourCam = Camera(glm::vec3(0.0f, 0.0f, 5.0f)); // Camera object
bool first_mouse = true; // For mouse movement
float lastX = SCR_WIDTH / 2.0f; // Last mouse X position
float lastY = SCR_HEIGHT / 2.0f; // Last mouse Y position

// Physics constants
const float gravity = -0.5f; // Gravity
const float groundY = -2.0f; // Ground level
const float damping = 0.5f; // Velocity damping
const int constraintIterations = 5; // Number of constraint iterations

// Function declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);

void simulateSoftBody(Model& model, float deltaTime, int numSubsteps) {
    float deltaTSub = deltaTime / numSubsteps; // Substep size

    for (int substep = 0; substep < numSubsteps; substep++) {
        // Update velocities and positions
        for (auto& mesh : model.meshes) {
            for (auto& vertex : mesh.vertices) {
                // Apply gravity to velocity
                vertex.Velocity += deltaTSub * glm::vec3(0.0f, gravity, 0.0f);

                // Save previous position
                vertex.PreviousPosition = vertex.Position;

                // Update position using velocity
                vertex.Position += deltaTSub * vertex.Velocity;
            }
        }

        // Solve constraints
        for (int i = 0; i < constraintIterations; i++) {
            for (auto& mesh : model.meshes) {
                // Solve distance constraints
                for (auto& constraint : mesh.distanceConstraints) {
                    mesh.solveDistanceConstraint(constraint, mesh.vertices, deltaTSub);
                }

                // Solve volume constraints
                for (auto& constraint : mesh.volumeConstraints) {
                    mesh.solveVolumeConstraint(constraint, mesh.vertices, mesh.tetIds, deltaTSub);
                }
            }
        }

        // Update velocities based on corrected positions
        for (auto& mesh : model.meshes) {
            for (auto& vertex : mesh.vertices) {
                vertex.Velocity = (vertex.Position - vertex.PreviousPosition) / deltaTSub;
            }
        }
    }

    // Handle collisions with the ground
    for (auto& mesh : model.meshes) {
        for (auto& vertex : mesh.vertices) {
            if (vertex.Position.y < groundY) {
                vertex.Position.y = groundY;
                vertex.Velocity.y *= -0.5f; // Bounce
                vertex.Velocity *= damping; // Dampen velocity
            }
        }
    }

    // Update VBOs
    for (auto& mesh : model.meshes) {
        glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data());
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

int main() {
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "slimeEngine", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Set up OpenGL viewport and callbacks
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Load shaders
    Shader ourShader(
        "C:/Users/zq/Documents/GitHub/slimeEngine/src/shaders/texture.vert",
        "C:/Users/zq/Documents/GitHub/slimeEngine/src/shaders/texture.frag"
    );

    // Load model
    stbi_set_flip_vertically_on_load(true);
    //Model testModel(FileSystem::getPath("assets/pudding/tetrapudding.obj"));
    Model testModel("C:/Users/zq/Desktop/school/CSD6/graphics/jiggle/tetrapuddingface.obj");

    // Set up point lights
    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f, 0.2f, 2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f, 2.0f, -12.0f),
        glm::vec3(0.0f, 0.0f, -3.0f)
    };

    // Model matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        if (deltaTime < 1e-6f) {
            deltaTime = 1e-6f;
        }
        lastFrame = currentFrame;

        int numSubsteps = 1;

        // Process input
        processInput(window);

        // Simulate soft body physics
        simulateSoftBody(testModel, deltaTime, numSubsteps);

        // Clear the screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use shader program
        ourShader.use();

        // Set up point light properties
        ourShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        ourShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        ourShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        ourShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        ourShader.setFloat("pointLights[0].constant", 1.0f);
        ourShader.setFloat("pointLights[0].linear", 0.09f);
        ourShader.setFloat("pointLights[0].quadratic", 0.032f);

        // Set up projection and view matrices
        glm::mat4 projection = glm::perspective(glm::radians(ourCam.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);
        glm::mat4 view = ourCam.GetViewMatrix();
        ourShader.setMat4("view", view);

        // Draw the model
        ourShader.setMat4("model", model);
        testModel.Draw(ourShader);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up and exit
    glfwTerminate();
    return 0;
}

// Callback for window resizing
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Callback for mouse movement
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
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

// Callback for mouse scroll
void scroll_callback(GLFWwindow* window, double xpos, double ypos) {
    ourCam.ProcessMouseScroll(static_cast<float>(ypos));
}

// Process keyboard input
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Camera movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        ourCam.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        ourCam.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        ourCam.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        ourCam.ProcessKeyboard(RIGHT, deltaTime);
}