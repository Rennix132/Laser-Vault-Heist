#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

#include "Shader.h"
#include "Mesh.h"
#include "Camera.h"
#include "Vault.h"

static const int WIDTH = 1280, HEIGHT = 720;
static Camera camera;
static Vault vault;
static bool flashlight = false;
static float lastX = WIDTH / 2.0f, lastY = HEIGHT / 2.0f;
static bool firstMouse = true;

static void mouseCallback(GLFWwindow*, double x, double y) {
    if (firstMouse) { lastX = (float)x; lastY = (float)y; firstMouse = false; }
    camera.look((float)x - lastX, lastY - (float)y);
    lastX = (float)x; lastY = (float)y;
}

static void resizeCallback(GLFWwindow*, int w, int h) { glViewport(0, 0, w, h); }

static void keyCallback(GLFWwindow* window, int key, int, int action, int) {
    if (action != GLFW_PRESS) return;
    switch (key) {
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, true); break;
        case GLFW_KEY_E: vault.interact(camera.position); break;
        case GLFW_KEY_F: flashlight = !flashlight; break;
        case GLFW_KEY_R:
            vault.reset();
            camera = Camera{};
            flashlight = false;
            break;
    }
}

static void processMovement(GLFWwindow* window, float dt) {
    const auto pressed = [window](int key) { return glfwGetKey(window, key) == GLFW_PRESS; };
    const int forward = pressed(GLFW_KEY_W) - pressed(GLFW_KEY_S);
    const int sideways = pressed(GLFW_KEY_D) - pressed(GLFW_KEY_A);
    if (forward && sideways) dt *= 0.70710678f;
    if (forward) camera.move(forward > 0 ? CamMove::Forward : CamMove::Backward, dt);
    if (sideways) camera.move(sideways > 0 ? CamMove::Right : CamMove::Left, dt);
}

int main() {
    if (!glfwInit()) { std::cerr << "Failed to init GLFW\n"; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Laser Vault Heist", nullptr, nullptr);
    if (!window) { std::cerr << "Failed to create window\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, resizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n";
        glfwTerminate();
        return -1;
    }
    glEnable(GL_DEPTH_TEST);

    // Scope so that GL objects are destroyed before the context is terminated
    {
        std::string dir = "./";
        Shader phong(dir + "shaders/phong.vert", dir + "shaders/phong.frag");
        Mesh cube = Mesh::createCube();


        float lastTime = (float)glfwGetTime();
        float fpsTimer = 0.0f; int frames = 0;

        while (!glfwWindowShouldClose(window)) {
            float now = (float)glfwGetTime();
            float dt = now - lastTime; lastTime = now;

            // FPS counter in the window title (requirement R7)
            frames++; fpsTimer += dt;
            if (fpsTimer >= 0.5f) {
                glfwSetWindowTitle(window, ("Laser Vault Heist | " + std::to_string((int)(frames / fpsTimer)) + " FPS | " +
                    vault.status(camera.position) + " | WASD / Mouse | F: light | R: reset").c_str());
                frames = 0; fpsTimer = 0.0f;
            }

            const glm::vec3 previous = camera.position;
            const float step = std::min(dt, 0.05f);
            processMovement(window, step);
            vault.update(step, previous, camera.position);

            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            if (width == 0 || height == 0) {
                glfwWaitEventsTimeout(0.05);
                continue;
            }
            glViewport(0, 0, width, height);

            glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            phong.use();
            glm::mat4 proj = glm::perspective(glm::radians(60.0f), static_cast<float>(width) / height, 0.1f, 100.0f);
            phong.setMat4("projection", proj);
            phong.setMat4("view", camera.getView());
            phong.setVec3("viewPos", camera.position);
            phong.setVec3("lampPos", glm::vec3(0, 2.8f, -3.0f));
            phong.setVec3("movingLightPos", glm::vec3(2.5f * std::sin(now), 2.7f, -9.0f));
            phong.setVec3("viewDirection", camera.front());
            phong.setInt("flashlight", flashlight);
            vault.draw(phong, cube);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    glfwTerminate();
    return 0;
}
