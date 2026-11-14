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
#include "EndMenu.h"
#include "GameAudio.h"

static const int WIDTH = 1280, HEIGHT = 720;
static Camera camera;
static Vault vault;
static float stepDistance = 0.0f;
static bool flashlight = false;
static bool safePreviewSeen = false;
static float menuProgress = 0.0f;
static float lastX = WIDTH / 2.0f, lastY = HEIGHT / 2.0f;
static bool firstMouse = true;
static bool cursorFree = true;

static bool safePreviewActive() {
    return !safePreviewSeen && !vault.finished() && vault.safeOpened();
}

static bool menuActive() {
    return vault.finished() || safePreviewActive();
}

static void setCursorFree(GLFWwindow* window, bool free) {
    cursorFree = free;
    firstMouse = true;
    glfwSetInputMode(window, GLFW_CURSOR, free ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

static void mouseCallback(GLFWwindow*, double x, double y) {
    if (cursorFree || menuActive()) return;
    if (firstMouse) { lastX = (float)x; lastY = (float)y; firstMouse = false; }
    camera.look((float)x - lastX, lastY - (float)y);
    lastX = (float)x; lastY = (float)y;
}

static void resizeCallback(GLFWwindow*, int w, int h) { glViewport(0, 0, w, h); }

static void restart(GLFWwindow* window) {
    vault.reset();
    stepDistance = 0.0f;
    safePreviewSeen = false;
    camera = Camera{};
    flashlight = false;
    menuProgress = 0.0f;
    setCursorFree(window, false);
}

static void acceptMenu(GLFWwindow* window) {
    if (vault.finished()) {
        restart(window);
        return;
    }
    safePreviewSeen = true;
    menuProgress = 0.0f;
    setCursorFree(window, false);
}

static void keyCallback(GLFWwindow* window, int key, int, int action, int) {
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, true);
    if (key == GLFW_KEY_TAB && !menuActive()) {
        setCursorFree(window, !cursorFree);
        return;
    }
    if (key == GLFW_KEY_R) restart(window);
    if (key == GLFW_KEY_ENTER && menuActive()) acceptMenu(window);
    if (cursorFree || menuActive()) return;
    if (key == GLFW_KEY_E) vault.interact(camera.position);
    if (key == GLFW_KEY_SPACE && vault.jump()) {
        static_cast<GameAudio*>(glfwGetWindowUserPointer(window))->jump();
        stepDistance = 0.0f;
    }
    if (key == GLFW_KEY_F) flashlight = !flashlight;
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int) {
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) return;
    if (menuActive()) {
        if (menuProgress >= 1.0f && menuButtonHovered(window)) acceptMenu(window);
    } else if (cursorFree) {
        setCursorFree(window, false);
    }
}

static void focusCallback(GLFWwindow* window, int focused) {
    if (!focused) setCursorFree(window, true);
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
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetFramebufferSizeCallback(window, resizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetWindowFocusCallback(window, focusCallback);
    setCursorFree(window, true);

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
        GameAudio audio;
        glfwSetWindowUserPointer(window, &audio);


        float lastTime = (float)glfwGetTime();
        float fpsTimer = 0.0f; int frames = 0;

        while (!glfwWindowShouldClose(window)) {
            float now = (float)glfwGetTime();
            float dt = now - lastTime; lastTime = now;

            // FPS counter in the window title (requirement R7)
            frames++; fpsTimer += dt;
            if (fpsTimer >= 0.5f) {
                glfwSetWindowTitle(window, ("Laser Vault Heist | " + std::to_string((int)(frames / fpsTimer)) + " FPS | " +
                    vault.status(camera.position) + (cursorFree && !menuActive()
                        ? " | Click or Tab to play" : " | Tab: release mouse")).c_str());
                frames = 0; fpsTimer = 0.0f;
            }

            const glm::vec3 previous = camera.position;
            const float step = std::min(dt, 0.05f);
            if (!cursorFree && !menuActive()) {
                const bool crouch = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                                    glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
                processMovement(window, crouch ? step * 0.65f : step);
                vault.update(step, previous, camera.position, crouch);
                if (vault.onGround() && !menuActive()) {
                    const glm::vec3 change = camera.position - previous;
                    stepDistance += std::hypot(change.x, change.z);
                    const float stride = crouch ? 0.72f : 1.12f;
                    if (stepDistance >= stride) {
                        audio.footstep(crouch);
                        stepDistance = 0.0f;
                    }
                }
            }
            if (menuActive()) {
                if (!cursorFree) setCursorFree(window, true);
                menuProgress = std::min(1.0f, menuProgress + step * 5.0f);
            }

            audio.update();

            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            if (width == 0 || height == 0) {
                glfwWaitEventsTimeout(0.01);
                continue;
            }
            glViewport(0, 0, width, height);

            glClearColor(0.015f, 0.018f, 0.028f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            phong.use();
            glm::mat4 proj = glm::perspective(glm::radians(60.0f), static_cast<float>(width) / height, 0.1f, 100.0f);
            phong.setMat4("projection", proj);
            phong.setMat4("view", camera.getView());
            phong.setVec3("viewPos", camera.position);
            phong.setVec3("viewDirection", camera.front());
            phong.setInt("flashlight", flashlight);
            vault.draw(phong, cube);

            if (menuActive()) {
                const auto screen = safePreviewActive() ? MenuScreen::SafeOpened
                                  : vault.won() ? MenuScreen::Victory : MenuScreen::Alarm;
                drawEndMenu(window, width, height, screen, menuProgress);
            }
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    glfwTerminate();
    return 0;
}
