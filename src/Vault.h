#pragma once

#include <array>
#include <glm/glm.hpp>

class Mesh;
class Shader;

class Vault {
public:
    void reset();
    void update(float dt, glm::vec3 previous, glm::vec3& player);
    void interact(const glm::vec3& player);
    void draw(const Shader& shader, const Mesh& cube) const;
    const char* status(const glm::vec3& player) const;

private:
    struct Beam { float z; float x; };
    std::array<Beam, 3> beams{{{0.5f, 0.0f}, {-3.5f, 0.0f}, {-7.0f, 0.0f}}};
    float elapsed = 0.0f;
    float doorAngle = 0.0f;
    bool hasGem = false;
    bool caught = false;
    bool escaped = false;
};
