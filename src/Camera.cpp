#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

Camera::Camera(glm::vec3 pos) : position(pos) { update(); }

glm::mat4 Camera::getView() const {
    return glm::lookAt(position, position + frontVec, glm::vec3(0, 1, 0));
}

void Camera::move(CamMove dir, float dt) {
    // walk on the XZ plane so looking up/down does not lift the player
    glm::vec3 flat = glm::normalize(glm::vec3(frontVec.x, 0.0f, frontVec.z));
    glm::vec3 right = glm::normalize(glm::cross(flat, glm::vec3(0, 1, 0)));
    float v = speed * dt;
    if (dir == CamMove::Forward)  position += flat * v;
    if (dir == CamMove::Backward) position -= flat * v;
    if (dir == CamMove::Left)     position -= right * v;
    if (dir == CamMove::Right)    position += right * v;
}

void Camera::look(float dx, float dy) {
    yaw += dx * sensitivity;
    pitch = std::clamp(pitch + dy * sensitivity, -89.0f, 89.0f);
    update();
}

void Camera::update() {
    float y = glm::radians(yaw), p = glm::radians(pitch);
    frontVec = glm::normalize(glm::vec3(std::cos(y) * std::cos(p), std::sin(p), std::sin(y) * std::cos(p)));
}
