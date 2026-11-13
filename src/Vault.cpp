#include "Vault.h"
#include "Mesh.h"
#include "Shader.h"

#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace {
constexpr float halfWidth = 4.0f;
constexpr float front = 7.0f;
constexpr float back = -12.0f;
constexpr float beamHalfLength = 2.05f;
constexpr float playerRadius = 0.22f;
const glm::vec3 safePosition(0.0f, 0.0f, -10.65f);
const glm::vec3 exitPosition(0.0f, 0.0f, 6.15f);
const glm::mat4 identity(1.0f);

bool near(glm::vec3 player, const glm::vec3& point, float radius) {
    player.y = 0.0f;
    return glm::distance(player, point) <= radius;
}
}

void Vault::reset() {
    *this = Vault{};
}

void Vault::update(float dt, glm::vec3 previous, glm::vec3& player) {
    player.x = std::clamp(player.x, -halfWidth + playerRadius, halfWidth - playerRadius);
    player.z = std::clamp(player.z, back + playerRadius, front - playerRadius);
    if (player.z < -9.8f && std::abs(player.x) < 1.0f) player = previous;
    if (caught || escaped) {
        player = previous;
        return;
    }

    elapsed += dt;
    if (hasGem) doorAngle = std::min(1.65f, doorAngle + dt * 2.5f);
    for (std::size_t i = 0; i < beams.size(); ++i) {
        auto& beam = beams[i];
        const float oldX = beam.x;
        beam.x = 1.75f * std::sin(elapsed * (0.85f + 0.15f * i) + 1.9f * i);
        const float minZ = std::min(player.z, previous.z) - playerRadius;
        const float maxZ = std::max(player.z, previous.z) + playerRadius;
        const float minX = std::min(player.x - beam.x, previous.x - oldX);
        const float maxX = std::max(player.x - beam.x, previous.x - oldX);
        if (minZ <= beam.z && maxZ >= beam.z &&
            minX <= beamHalfLength + playerRadius && maxX >= -beamHalfLength - playerRadius) {
            caught = true;
        }
    }
}

void Vault::interact(const glm::vec3& player) {
    if (caught || escaped) return;
    if (!hasGem && near(player, safePosition, 1.9f)) hasGem = true;
    if (hasGem && near(player, exitPosition, 1.15f)) escaped = true;
}

const char* Vault::status(const glm::vec3& player) const {
    if (caught) return "Alarm! R: retry";
    if (escaped) return "Heist complete! R: replay";
    if (hasGem && near(player, exitPosition, 1.15f)) return "E: escape";
    if (hasGem) return "Gem secured - return to the green exit";
    if (near(player, safePosition, 1.9f)) return "E: take the gem";
    return "Reach the vault - avoid the red beams";
}

void Vault::draw(const Shader& shader, const Mesh& cube) const {
    const auto box = [&](glm::vec3 position, glm::vec3 size, glm::vec3 color,
                         float emission = 0.0f, bool tiles = false,
                         const glm::mat4& parent = identity) {
        shader.setMat4("model", glm::scale(glm::translate(parent, position), size));
        shader.setVec3("objectColor", color);
        shader.setFloat("emission", emission);
        shader.setInt("tiled", tiles);
        cube.draw();
    };

    const glm::vec3 wall(0.12f, 0.17f, 0.24f);
    const glm::vec3 metal(0.27f, 0.34f, 0.42f);
    const glm::vec3 cyan(0.16f, 0.72f, 0.98f);
    const glm::vec3 red(1.0f, 0.045f, 0.09f);
    const glm::vec3 green(0.18f, 0.95f, 0.48f);
    box({0, -0.12f, (front + back) / 2}, {8, 0.24f, front - back}, metal, 0, true);
    box({0, 3.1f, (front + back) / 2}, {8, 0.2f, front - back}, wall);
    for (float x : {-halfWidth, halfWidth}) {
        box({x, 1.5f, (front + back) / 2}, {0.2f, 3, front - back}, wall);
        for (float z = -10; z <= 6; z += 4) {
            box({x * 0.96f, 1.5f, z}, {0.28f, 3, 0.42f}, metal);
            box({x * 0.91f, 2.3f, z}, {0.06f, 0.75f, 0.13f}, cyan, 0.8f);
        }
    }
    box({0, 1.5f, front}, {8, 3, 0.2f}, wall);
    box({0, 1.5f, back}, {8, 3, 0.2f}, wall);
    box({0, 1.05f, 6.88f}, {1.65f, 2.1f, 0.12f}, metal);
    box({0, 2.3f, 6.78f}, {1.65f, 0.12f, 0.12f}, green, 0.9f);

    for (const auto& beam : beams) {
        box({beam.x, 1.15f, beam.z}, {2 * beamHalfLength, 0.065f, 0.065f}, red, 1.0f);
        for (float side : {-1.0f, 1.0f}) {
            box({beam.x + side * beamHalfLength, 1.15f, beam.z},
                {0.16f, 0.16f, 0.16f}, red, 0.65f);
        }
    }

    box(safePosition + glm::vec3(0, 0.82f, 0), {1.8f, 1.64f, 1.25f}, metal);
    const auto hinge = glm::rotate(glm::translate(identity,
        safePosition + glm::vec3(-0.9f, 0.82f, 0.74f)), -doorAngle, glm::vec3(0, 1, 0));
    box({0.9f, 0, 0}, {1.8f, 1.64f, 0.15f}, wall, 0, false, hinge);
    box({1.38f, 0, 0.13f}, {0.38f, 0.12f, 0.15f}, cyan, 0.6f, false, hinge);
    box({0, 2.26f, -10.6f}, {1.6f, 0.1f, 0.1f}, hasGem ? green : cyan, 0.8f);
    if (!hasGem) {
        const auto gem = glm::rotate(glm::translate(identity, safePosition + glm::vec3(0, 1.98f, 0)),
                                    elapsed, glm::vec3(0, 1, 0));
        box({0, 0, 0}, {0.3f, 0.3f, 0.3f}, cyan, 0.65f, false,
            glm::rotate(gem, 0.78f, glm::vec3(1, 0, 1)));
    }
}
