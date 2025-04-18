#include <imgui/headers/imgui.h>
#include <imgui/headers/imgui_impl_glfw.h>
#include <imgui/headers/imgui_impl_opengl3.h>

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

//#include "structs/debugging.h"

#include "structs/Hit.h"
#include "structs/Ray.h"

#include "structs/Model.h"

#include <learnopengl/filesystem.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Window dimensions
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Global variables
float deltaTime = 0.0f; // Time between frames
float lastFrame = 0.0f; // Time of the last frame

// Camera
//float yaw = -90.0f;
//float pitch = -20.0f;
//Camera ourCam = Camera(glm::vec3(0.0f, -4.0f, 5.0f)); // Camera object
Camera ourCam = Camera(glm::vec3(0.0f, -2.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -20.0f);
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
int substeps = 30; //more = smoother, 10 to 30 is good
bool reset = true;

glm::vec3 mouse_offset = { 0, 0, 0 };
bool grab = false;
//bool reset = false;
Particle* grabbed_particle = nullptr;
Hit* h = new Hit();
bool cursor = false;

//bool dragging = false;
//int selectedVertexIndex = -1;

// Function declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow* window, double xpos, double ypos);
//void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow* window);

void loadObject(const std::string& name) {
    std::string basePath = "assets/" + name + "/" + name;

    testModel = new Model(FileSystem::getPath(basePath + ".obj"));

    testModel->meshes[0].initSoftBody(
        FileSystem::getPath(basePath + ".1.node"),
        FileSystem::getPath(basePath + ".1.ele"),
        mass,
        edge_compliance,
        volume_compliance
    );
}

Mesh* intersection(Camera& c, Model& m, Hit& h) {
    Mesh* hitMesh = nullptr;
    bool intersect = false;
    Ray r(c.Position, c.Front);

    if (m.meshes.empty()) {
        std::cerr << "Model has no meshes!" << std::endl;
        return nullptr;
    }

    for (Mesh& mesh : m.meshes) {
        float t0 = h.getT();
        mesh.intersect(r, h, 0.0f);
        float t1 = h.getT();
        if (t1 < t0) {
            hitMesh = &mesh;
            intersect = true;
        }
    }
    return hitMesh;
}


Particle* findPointRT(Camera& c, Hit& h, Mesh& hitMesh) {
    float t = h.getT();
    glm::vec3 intersection_point = c.Position + t * c.Front;
    Particle* nearest_particle = &hitMesh.particles[0];
    float min_distance = glm::length(intersection_point - nearest_particle->pos);

    for (Particle& p : hitMesh.particles) {
        float new_distance = glm::length(intersection_point - p.pos);
        if (new_distance < min_distance) {
            nearest_particle = &p;
            min_distance = new_distance;
        }
    }
    return nearest_particle;
}

void reset_grabbed() {
    grabbed_particle->inv_mass = grabbed_particle->mass;
    grabbed_particle->velocity = glm::vec3(0.0f);
    grabbed_particle = nullptr;
    grab = false;
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
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    //glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

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

    std::vector<std::string> availableObjects = { "pudding", "sphere"};

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

        if (ImGui::Button("Reset")) {
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
            grab = false;
            reset = false;
        }

        if (grab) {
            if (grabbed_particle == nullptr) {
                Mesh* hitMesh = intersection(ourCam, *testModel, *h);
                if (hitMesh != nullptr) {
                    grabbed_particle = findPointRT(ourCam, *h, *hitMesh);
                }
            }
            else {
                grabbed_particle->inv_mass = 0.0f;
                grabbed_particle->pos = ourCam.Position + h->getT() * ourCam.Front;
            }
        }
        else {
            if (grabbed_particle != nullptr) {
                reset_grabbed();
                h = new Hit();
            }
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
    if (!cursor) {
        ourCam.ProcessMouseMovement(xoffset, yoffset, true);
    }
    mouse_offset.x = xoffset;
    mouse_offset.y = yoffset;
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


    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        grab = true;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        grab = false;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        reset = true;
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
        cursor = true;
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        cursor = false;
    }
    if (cursor) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

