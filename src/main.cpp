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

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

glm::vec3 gravity = { 0, -10, 0 };
int substeps = 3;

//float mixValue = 0.2;

glm::mat4 view;

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f;

//Camera ourCam = Camera();
Camera ourCam = Camera(glm::vec3(0.0f, -2.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -20.0f);
bool first_mouse = true;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

float edge_compliance = 0.01f; // higher = more jiggly, 0.01 is good
float volume_compliance = 0.001f;
float mass = 0.1f;

glm::vec3 mouse_offset = { 0, 0, 0 };
bool grab = false;
bool reset = true;
Particle* grabbed_particle = nullptr;
Hit* h = new Hit();

bool cursor = false;

// Create callback function for resizing window
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

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

void scroll_callback(GLFWwindow* window, double xpos, double ypos) {
    ourCam.ProcessMouseScroll(static_cast<float>(ypos));
}

// Processing Input
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
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
    //if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    //    ourCam.ProcessKeyboard(UP, deltaTime);
    //}
    //if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
    //    ourCam.ProcessKeyboard(DOWN, deltaTime);
    //}
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

Model loadObject(const std::string& name) {
    std::string basePath = "assets/" + name + "/" + name;

    Model testModel(FileSystem::getPath(basePath + ".obj"));

    testModel.meshes[0].initSoftBody(FileSystem::getPath(basePath + ".1.node"),
        FileSystem::getPath(basePath + ".1.ele"),
        mass, edge_compliance, volume_compliance);
    return testModel;
}

Mesh* intersection(Camera& c, Model& m, Hit& h) {
    Mesh* hitMesh = nullptr;
    bool intersect = false;
    Ray r(c.Position, c.Front);
    for (Mesh& mesh : m.meshes) {
        // buffer var to check when to update mesh
        float t0 = h.getT();
        mesh.intersect(r, h, 0.0f);
        float t1 = h.getT();
        // update hitMesh if t is updated
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
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window object
    GLFWwindow* window =
        glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "slimeEngine", NULL, NULL);

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
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // tell GLFW to call this function on every window resize
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f, 0.2f, 2.0f), glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f, 2.0f, -12.0f), glm::vec3(1.0f, 2.0f, 3.0f) };


    glEnable(GL_DEPTH_TEST);

    stbi_set_flip_vertically_on_load(true);

    // Load shaders
    Shader ourShader(
        "C:/Users/zq/Documents/GitHub/slimeEngine/src/shaders/texture2.vert",
        "C:/Users/zq/Documents/GitHub/slimeEngine/src/shaders/texture2.frag"
    );
    ourShader.use();

    ourShader.setVec3("pointLights[0].position", 0.0f, 5.0f, 0.0f);
    ourShader.setVec3("pointLights[0].ambient", 0.1f, 0.1f, 0.1f);
    ourShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
    ourShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    ourShader.setFloat("pointLights[0].constant", 1.0f);
    ourShader.setFloat("pointLights[0].linear", 0.09f);
    ourShader.setFloat("pointLights[0].quadratic", 0.032f);

    ourCam.Position = { 0, 1, 5.0f };

    std::vector<std::string> availableObjects = { "pudding", "sphere" };

    int object_index = 0; // change this to change object used
    Model testModel = loadObject(availableObjects[object_index]);

    Model floor(FileSystem::getPath("assets/floor/floor.obj"));

    glEnable(GL_BLEND); // you enable blending function
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize ImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");


    // Render loop
    while (!glfwWindowShouldClose(window)) {

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // Inputs
        processInput(window);
        // Rendering Commands
        //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // use the program

        ourShader.use();

        ourShader.setVec3("pointLights[0].position", 0.0f, 5.0f, 0.0f);
        ourShader.setVec3("pointLights[0].ambient", 0.3f, 0.3f, 0.3f);
        ourShader.setVec3("pointLights[0].diffuse", 0.7f, 0.7f, 0.7f);
        ourShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        ourShader.setFloat("pointLights[0].constant", 1.0f);
        ourShader.setFloat("pointLights[0].linear", 0.09f);
        ourShader.setFloat("pointLights[0].quadratic", 0.032f);

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
        glm::mat4 floor_model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
        ourShader.setMat4("model", floor_model);
        floor.Draw(ourShader);
        testModel.meshes[0].update(deltaTime, substeps, gravity);

        if (reset) {
            testModel.meshes[0].reset();
            grab = false;
            reset = false;
        }

        if (grab) {
            if (grabbed_particle == nullptr) {
                Mesh* hitMesh = intersection(ourCam, testModel, *h);
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

        // Tell OpenGL a new frame is about to begin
            ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        // ImGUI window creation
        ImGui::Begin("Soft Body Parameters", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        // Sliders for parameters
        ImGui::SliderFloat("Edge compliance", &testModel.meshes[0].edge_compliance, 0.005f, 0.05f);
        ImGui::SliderFloat("Volume compliance", &testModel.meshes[0].volume_compliance, 0.001f, 0.2f);
        ImGui::SliderInt("Substeps", &substeps, 1, 50);

        if (ImGui::Button("Reset")) {
            reset = true;
        }
        if (ImGui::Button("Lift")) {
            // Optional: do something on click
        }

        // Detect if "Lift" button is being held down
        if (ImGui::IsItemActive()) {
            for (int i = 0; i < testModel.meshes[0].particles.size(); ++i) {
                testModel.meshes[0].particles[i].pos += glm::vec3(0, 0.01f, 0);
            }
        }

        // Ends the window
        ImGui::End();

        // Renders the ImGUI elements
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // Deletes all ImGUI instances
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}