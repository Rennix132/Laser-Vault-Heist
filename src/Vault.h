#pragma once

#include <glm/glm.hpp>

class Mesh;
class Shader;

class Vault {
public:
    void reset();
    void update(float dt, glm::vec3 previous, glm::vec3& player, bool crouch);
    void interact(glm::vec3& player);
    bool jump();
    bool onGround() const { return grounded; }
    bool finished() const { return caught || escaped; }
    bool safeOpened() const { return doorAngle >= openDoorAngle; }
    bool won() const { return escaped; }
    void draw(const Shader& shader, const Mesh& cube) const;
    const char* status(const glm::vec3& player) const;

private:
    static constexpr float openDoorAngle = 1.65f;
    float feet = 0.0f;
    float verticalSpeed = 0.0f;
    bool grounded = true;
    float elapsed = 0.0f;
    float doorAngle = 0.0f;
    bool hasGem = false;
    bool caught = false;
    bool escaped = false;
};
