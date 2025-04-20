#include <imgui/headers/imgui.h>
#include <imgui/headers/imgui_impl_glfw.h>
#include <imgui/headers/imgui_impl_opengl3.h>

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
float yaw = -90.0f;
float pitch = -20.0f;
//Camera ourCam = Camera(glm::vec3(0.0f, -4.0f, 5.0f)); // Camera object
Camera ourCam = Camera(glm::vec3(0.0f, -1.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), yaw, pitch);
bool first_mouse = true; // For mouse movement
float lastX = SCR_WIDTH / 2.0f; // Last mouse X position
float lastY = SCR_HEIGHT / 2.0f; // Last mouse Y position

Model* testModel;
Model* floorModel;

float mass = 0.01f; // higher = more jiggly, 0.01 is good

//compliance is 0 to 10
//edge *50

//add this 4 things to imgui
float edge_compliance = 0.01f; // higher = more jiggly, 0.01 is good
float volume_compliance = 0.1f;
int substeps = 10; //more = smoother, 10 to 30 is good
bool reset = true;

//bool dragging = false;
//int selectedVertexIndex = -1;

// Function declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow* window);

void loadObject(const std::string& name) {
    std::string basePath = "assets/" + name + "/" + name;

    testModel = new Model(FileSystem::getPath(basePath + ".stl"));

    testModel->meshes[0].initSoftBody(
        FileSystem::getPath(basePath + ".1.node"),
        FileSystem::getPath(basePath + ".1.ele"),
        mass,
        edge_compliance,
        volume_compliance
    );
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
    //comment this to stop camera movement: can toggle with ctrl?
    //glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Load shaders
    Shader ourShader(
        "C:/Users/zq/Documents/GitHub/slimeEngine/src/shaders/texture2.vert",
        "C:/Users/zq/Documents/GitHub/slimeEngine/src/shaders/texture2.frag"
    );

    // Load model
    stbi_set_flip_vertically_on_load(true);

    Model* floorModel = new Model(FileSystem::getPath("assets/floor/floor.obj"));

    std::vector<std::string> availableObjects = { "tetrahedron", "cube", "pudding", "sphere", "bunny", };

    int object_index = 0; //change this to change object used
    loadObject(availableObjects[object_index]);

    //loadObject("sphere");


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

    // Initialize ImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        /*     if (deltaTime < 1e-5f) {
                 deltaTime = 1e-5f;
             }*/
             //deltaTime = 1.0f / 60.0f;
        lastFrame = currentFrame;

        // Process input
        processInput(window);

        // Clear the screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Tell OpenGL a new frame is about to begin
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        // ImGUI window creation
        ImGui::Begin("Soft Body Parameters", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        // Sliders for parameters
        ImGui::SliderFloat("Edge compliance", &testModel->meshes[0].edge_compliance, 0.01f, 0.2f);
        ImGui::SliderFloat("Volume compliance", &testModel->meshes[0].volume_compliance, 0.01f, 0.2f);
        ImGui::SliderInt("Substeps", &substeps, 1, 50);

        if (ImGui::Button("Drop")) {
            reset = true;
        }
        if (ImGui::Button("Lift")) {
            // Optional: do something on click
        }

        // Detect if "Lift" button is being held down
        if (ImGui::IsItemActive()) {
            for (int i = 0; i < testModel->meshes[0].particles.size(); ++i) {
                testModel->meshes[0].particles[i].pos += glm::vec3(0, 0.01f, 0);
            }
        }

        //if (ImGui::Button("Push left")) {
        //    testModel->meshes[0].particles.back().pos += glm::vec3(-0.8f, 0, 0);
        //}
        //if (ImGui::Button("Push right")) {
        //    testModel->meshes[0].particles.back().pos += glm::vec3(0.8f, 0, 0);
        //}

        // Ends the window
        ImGui::End();

        // Use shader program
        ourShader.use();

        // Set up point light properties
        ourShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        ourShader.setVec3("pointLights[0].ambient", 0.1f, 0.1f, 0.1f);
        ourShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        ourShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        ourShader.setFloat("pointLights[0].constant", 1.0f);
        ourShader.setFloat("pointLights[0].linear", 0.09f);
        ourShader.setFloat("pointLights[0].quadratic", 0.032f);

        ourShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        ourShader.setVec3("pointLights[1].ambient", 0.1f, 0.1f, 0.1f);
        ourShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
        ourShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        ourShader.setFloat("pointLights[1].constant", 1.0f);
        ourShader.setFloat("pointLights[1].linear", 0.09f);
        ourShader.setFloat("pointLights[1].quadratic", 0.032f);

        // Set up projection and view matrices
        glm::mat4 projection = glm::perspective(glm::radians(ourCam.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);
        glm::mat4 view = ourCam.GetViewMatrix();
        ourShader.setMat4("view", view);

        // Draw the model
        ourShader.setMat4("model", model);

        //if (currentFrame > 5) {

        glm::vec3 gravity = { 0, -10, 0 };

        testModel->meshes[0].update(deltaTime, substeps, gravity);

        //testModel->meshes[0].updateCompliance(edge_compliance, volume_compliance);
        if (reset) {
            testModel->meshes[0].reset();
            reset = false;
        }
        testModel->Draw(ourShader);

        glm::mat4 floorModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.0f, 0.0f));
        ourShader.setMat4("model", floorModelMatrix);
        floorModel->Draw(ourShader);

        // Renders the ImGUI elements
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Deletes all ImGUI instances
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            float ndcX = (2.0f * xpos / SCR_WIDTH) - 1.0f;
            float ndcY = 1.0f - (2.0f * ypos / SCR_HEIGHT);

            glm::vec2 mousePos = glm::vec2(ndcX, ndcY);
            //cout << "Mouse Position: " << mousePos.x << ", " << mousePos.y << endl;  // Debugging

            float threshold = 0.05f;

            //cout << "Mouse button pressed" << endl;

            //glm::vec3 vertexPosition = testModel->meshes[0].vertices[0].Position;
            //testModel->meshes[0].particles[0].pos += glm::vec3(0, 0.1, 0); // I guess that works
            //cout << testModel->meshes[0].particles[0].pos.y << endl;

            //for (int i = 0; i < testModel->meshes[0].particles.size(); ++i) {
            //    glm::vec2 v2D = glm::vec2(testModel->meshes[0].particles[i].pos.x,
            //        testModel->meshes[0].particles[i].pos.y);
            //    //cout << "Checking vertex " << i << ": " << v2D.x << ", " << v2D.y << endl;  // Debugging
            //    if (glm::distance(v2D, mousePos) < threshold) {
            //        dragging = true;
            //        cout << "dragging";
            //        selectedVertexIndex = i;
            //        break;
            //    }
            //}
        }
        else if (action == GLFW_RELEASE) {
            //dragging = false;
        }
    }
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


    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        if (testModel->meshes[0].edge_compliance > 0.01) {
            testModel->meshes[0].edge_compliance -= 0.0001;
            cout << "Edge compliance: " << testModel->meshes[0].edge_compliance << endl;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
        testModel->meshes[0].edge_compliance += 0.0001;
        cout << "Edge compliance: " << testModel->meshes[0].edge_compliance << endl;
    }

    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        if (volume_compliance >= 0.01) {
            testModel->meshes[0].volume_compliance -= 0.001;
            cout << "Volume compliance: " << testModel->meshes[0].volume_compliance << endl;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
        testModel->meshes[0].volume_compliance += 0.001;
        cout << "Volume compliance: " << testModel->meshes[0].volume_compliance << endl;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        if (substeps >= 1) {
            substeps -= 1;
            cout << "Substeps: " << substeps << endl;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
        substeps += 1;
        cout << "Substeps: " << substeps << endl;
    }

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        testModel->meshes[0].particles.back().pos += glm::vec3(-0.1f, 0, 0);
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        testModel->meshes[0].particles.back().pos += glm::vec3(0.1f, 0, 0);
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        //reset();
        reset = true;
        //testModel->meshes[0].edge_compliance = 0.01;

    }

    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
        for (int i = 0; i < testModel->meshes[0].particles.size(); ++i) {
            testModel->meshes[0].particles[i].pos += glm::vec3(0, 0.01f, 0);
        }
    }
}