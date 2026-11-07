#pragma once
#include <glm/glm.hpp>

enum class CamMove { Forward, Backward, Left, Right };

class Camera {
public:
    glm::vec3 position;
    float yaw = -90.0f, pitch = 0.0f;
    float speed = 3.0f, sensitivity = 0.1f;

    Camera(glm::vec3 pos = glm::vec3(0.0f, 1.7f, 5.0f));

    glm::mat4 getView() const;
    void move(CamMove dir, float dt);
    void look(float dx, float dy);
    glm::vec3 front() const { return frontVec; }

private:
    glm::vec3 frontVec{0, 0, -1};
    void update();
};
