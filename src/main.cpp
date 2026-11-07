#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>

#include "Shader.h"
#include "Mesh.h"
#include "Camera.h"

static const int WIDTH = 1280, HEIGHT = 720;
static Camera camera;
static float lastX = WIDTH / 2.0f, lastY = HEIGHT / 2.0f;
static bool firstMouse = true;

static void mouseCallback(GLFWwindow*, double x, double y) {
    if (firstMouse) { lastX = (float)x; lastY = (float)y; firstMouse = false; }
    camera.look((float)x - lastX, lastY - (float)y);
    lastX = (float)x; lastY = (float)y;
}

static void resizeCallback(GLFWwindow*, int w, int h) { glViewport(0, 0, w, h); }

static void processInput(GLFWwindow* win, float dt) {
    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(win, true);
    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) camera.move(CamMove::Forward, dt);
    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) camera.move(CamMove::Backward, dt);
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) camera.move(CamMove::Left, dt);
    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) camera.move(CamMove::Right, dt);
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
    glfwSetFramebufferSizeCallback(window, resizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n"; return -1;
    }
    glEnable(GL_DEPTH_TEST);

    // Scope so that GL objects are destroyed before the context is terminated
    {
        std::string dir = ASSET_DIR;
        Shader phong(dir + "shaders/phong.vert", dir + "shaders/phong.frag");
        Mesh cube = Mesh::createCube();
        Mesh floorMesh = Mesh::createPlane(10.0f, 1.0f);

        float lastTime = (float)glfwGetTime();
        float fpsTimer = 0.0f; int frames = 0;

        while (!glfwWindowShouldClose(window)) {
            float now = (float)glfwGetTime();
            float dt = now - lastTime; lastTime = now;

            // FPS counter in the window title (requirement R7)
            frames++; fpsTimer += dt;
            if (fpsTimer >= 0.5f) {
                glfwSetWindowTitle(window, ("Laser Vault Heist | FPS: " + std::to_string((int)(frames / fpsTimer))).c_str());
                frames = 0; fpsTimer = 0.0f;
            }

            processInput(window, dt);

            glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            phong.use();
            glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);
            phong.setMat4("projection", proj);
            phong.setMat4("view", camera.getView());
            phong.setVec3("viewPos", camera.position);
            phong.setVec3("lightPos", glm::vec3(0.0f, 4.0f, 0.0f));
            phong.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));

            // floor
            phong.setMat4("model", glm::mat4(1.0f));
            phong.setVec3("objectColor", glm::vec3(0.3f, 0.3f, 0.35f));
            floorMesh.draw();

            // test cube (will become the safe later)
            glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f));
            m = glm::rotate(m, now, glm::vec3(0, 1, 0));
            phong.setMat4("model", m);
            phong.setVec3("objectColor", glm::vec3(0.8f, 0.6f, 0.1f));
            cube.draw();

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    glfwTerminate();
    return 0;
}
