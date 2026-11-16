#include "Vault.h"
#include "Mesh.h"
#include "Shader.h"

#include <algorithm>
#include <array>
#include <string>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace {
constexpr float halfWidth = 4.0f;
constexpr float front = 7.0f;
constexpr float back = -12.0f;
constexpr float jumpBeamZ = 2.0f;
constexpr float crouchBeamZ = -1.0f;
constexpr float jumpBeamY = 0.36f;
constexpr float crouchBeamY = 1.30f;
constexpr float neonX = 3.76f;
constexpr float neonY = 2.85f;
constexpr float climbZ = -5.0f;
constexpr float climbHalfDepth = 0.8f;
constexpr float climbHeight = 1.25f;
constexpr float ladderX = 0.0f;
constexpr float ladderWidth = 1.1f;
constexpr float beamThickness = 0.08f;
constexpr float playerRadius = 0.22f;
const glm::vec3 safePosition(0.0f, 0.0f, -10.65f);
const glm::vec3 exitPosition(0.0f, 0.0f, 6.15f);
const glm::mat4 identity(1.0f);

struct Laser { glm::vec3 center, size; };
constexpr int gridLevel = 4;
constexpr int maxLasers = 8;
constexpr float gridGapHalfWidth = 1.05f;

int laserCountFor(int level) {
    return level == gridLevel ? maxLasers : level + 1;
}

float gridPosition(float time) {
    constexpr float nearEnd = 4.4f;
    constexpr float farEnd = -3.0f;
    constexpr float length = nearEnd - farEnd;
    const float travel = std::fmod(time * 3.0f, 2.0f * length);
    return farEnd + length - std::abs(travel - length);
}

std::array<Laser, maxLasers> lasersFor(int level, float time) {
    if (level == gridLevel) {
        const float z = gridPosition(time);
        const float wingWidth = halfWidth - gridGapHalfWidth;
        const float wingCenter = gridGapHalfWidth + wingWidth / 2;
        return {{
            {{0, 0.50f, z}, {8, beamThickness, beamThickness}},
            {{0, 2.80f, z}, {8, beamThickness, beamThickness}},
            {{-gridGapHalfWidth, 1.7f, z}, {beamThickness, 3.4f, beamThickness}},
            {{ gridGapHalfWidth, 1.7f, z}, {beamThickness, 3.4f, beamThickness}},
            {{-wingCenter, 0.95f, z}, {wingWidth, beamThickness, beamThickness}},
            {{ wingCenter, 0.95f, z}, {wingWidth, beamThickness, beamThickness}},
            {{-wingCenter, 1.65f, z}, {wingWidth, beamThickness, beamThickness}},
            {{ wingCenter, 1.65f, z}, {wingWidth, beamThickness, beamThickness}}
        }};
    }
    const float patrol = std::sin(time * (level == 3 ? 1.55f : 1.05f));
    const float sweep = std::sin(time * 1.7f);
    const Laser low{{0, jumpBeamY, jumpBeamZ}, {8, beamThickness, beamThickness}};
    const Laser high{{0, crouchBeamY, crouchBeamZ}, {8, beamThickness, beamThickness}};
    const Laser vertical{{2.65f * sweep, 1.7f, 0.55f}, {beamThickness, 3.4f, beamThickness}};
    const Laser moving{{0, crouchBeamY, -1.85f + 0.85f * patrol}, {8, beamThickness, beamThickness}};
    const Laser movingColumn{{-1.65f + 0.65f * sweep, 1.7f, -7.75f + 1.05f * patrol},
                             {beamThickness, 3.4f, beamThickness}};
    if (level == 1) return {low, high, {}, {}};
    if (level == 2) return {low, vertical, moving, {}};
    return {low, vertical, moving, movingColumn};
}

bool nearLadder(const glm::vec3& player) {
    const float edgeDistance = std::abs(player.z - climbZ) - climbHalfDepth;
    return std::abs(player.x - ladderX) <= ladderWidth / 2 &&
           edgeDistance >= 0.0f && edgeDistance <= 0.85f;
}

bool near(glm::vec3 player, const glm::vec3& point, float radius) {
    player.y = 0.0f;
    return glm::distance(player, point) <= radius;
}
}

void Vault::reset(int nextLevel) {
    *this = Vault{};
    level = nextLevel;
}

bool Vault::jump() {
    if (!grounded || finished()) return false;
    verticalSpeed = 4.5f;
    grounded = false;
    return true;
}

void Vault::update(float dt, glm::vec3 previous, glm::vec3& player, bool crouch) {
    if (finished()) return;
    player.x = std::clamp(player.x, -halfWidth + playerRadius, halfWidth - playerRadius);
    player.z = std::clamp(player.z, back + playerRadius, front - playerRadius);
    if (player.z < -9.8f && std::abs(player.x) < 1.0f) player = previous;
    if (feet < climbHeight && std::abs(player.z - climbZ) < climbHalfDepth + playerRadius) {
        player.z = previous.z;
    }

    const float floor = std::abs(player.z - climbZ) < climbHalfDepth + playerRadius
                        ? climbHeight : 0.0f;
    const float oldFeet = feet;
    verticalSpeed -= 12.0f * dt;
    feet = std::max(floor, feet + verticalSpeed * dt);
    grounded = feet <= floor && verticalSpeed <= 0.0f;
    if (grounded) verticalSpeed = 0.0f;
    const float eyeHeight = crouch ? 0.90f : 1.70f;
    player.y = feet + eyeHeight;

    const auto previousLasers = lasersFor(level, elapsed);
    elapsed += dt;
    const auto currentLasers = lasersFor(level, elapsed);
    for (int i = 0; i < laserCountFor(level); ++i) {
        const auto& laser = currentLasers[i];
        const auto half = laser.size * 0.5f;
        const glm::vec3 oldOffset = previous - previousLasers[i].center;
        const glm::vec3 newOffset = player - laser.center;
        const auto nearAxis = [](float a, float b, float limit) {
            return std::min(a, b) <= limit && std::max(a, b) >= -limit;
        };
        if (nearAxis(oldOffset.x, newOffset.x, half.x + playerRadius) &&
            nearAxis(oldOffset.z, newOffset.z, half.z + playerRadius) &&
            std::min(feet - laser.center.y, oldFeet - previousLasers[i].center.y) <= half.y &&
            std::max(feet - laser.center.y, oldFeet - previousLasers[i].center.y) + eyeHeight + 0.12f >= -half.y) {
            caught = true;
            break;
        }
    }
    if (hasGem) doorAngle = std::min(openDoorAngle, doorAngle + dt * 2.5f);
}

void Vault::interact(glm::vec3& player) {
    if (finished()) return;
    if (grounded && feet == 0.0f && nearLadder(player)) {
        player.z = climbZ + std::copysign(climbHalfDepth - 0.18f, player.z - climbZ);
        feet = climbHeight;
        player.y = feet + 1.70f;
        return;
    }
    if (!hasGem && near(player, safePosition, 1.9f)) hasGem = true;
    if (hasGem && near(player, exitPosition, 1.15f)) escaped = true;
}

const char* Vault::status(const glm::vec3& player) const {
    if (caught) return "Caught";
    if (escaped) return "Level complete";
    if (hasGem && near(player, exitPosition, 1.15f)) return "E: escape";
    if (!hasGem && near(player, safePosition, 1.9f)) return "E: take the gem";
    if (grounded && feet == 0.0f && nearLadder(player)) return "E: climb the ladder";
    if (level == gridLevel) return std::abs(player.x) > 0.75f
        ? "A OR D: LINE UP WITH THE CENTER GAP" : "SPACE: JUMP AS THE GRID GETS CLOSE";
    if (std::abs(player.z - jumpBeamZ) < 1.6f) return "SPACE: jump over the low beam";
    if (level == 1 && std::abs(player.z - crouchBeamZ) < 1.6f) return "CTRL: crouch under the high beam";
    if (level > 1 && std::abs(player.z + 1.85f) < 1.7f) return "CTRL: stay low under the moving laser";
    if (level > 1 && (std::abs(player.z - 0.55f) < 1.05f ||
                      (level == 3 && std::abs(player.z + 7.75f) < 1.35f)))
        return "A or D: dodge the vertical laser";
    return hasGem ? "Return to the green exit" : "Reach the vault and take the gem";
}

void Vault::draw(const Shader& shader, const Mesh& cube) const {
    const auto lasers = lasersFor(level, elapsed);
    shader.setInt("laserCount", laserCountFor(level));
    for (int i = 0; i < laserCountFor(level); ++i) {
        const std::string index = "[" + std::to_string(i) + "]";
        shader.setVec3("laserCenters" + index, lasers[i].center);
        shader.setVec3("laserExtents" + index, lasers[i].size * 0.5f);
    }
    shader.setVec3("neonStrip", {neonX, neonY, 0});
    shader.setFloat("neonNearZ", front - 0.35f);
    shader.setFloat("neonFarZ", back + 0.35f);

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
    box({0, 3.5f, (front + back) / 2}, {8, 0.2f, front - back}, wall);
    for (float x : {-halfWidth, halfWidth}) {
        box({x, 1.7f, (front + back) / 2}, {0.2f, 3.4f, front - back}, wall);
        box({std::copysign(neonX, x), neonY, (front + back) / 2},
            {0.09f, 0.07f, front - back - 0.7f}, cyan, 1.0f);
        for (float z = -10; z <= 6; z += 4) {
            box({x * 0.96f, 1.5f, z}, {0.28f, 3, 0.42f}, metal);
            box({x * 0.91f, 2.3f, z}, {0.06f, 0.75f, 0.13f}, cyan, 0.8f);
        }
    }
    box({0, 1.5f, front}, {8, 3, 0.2f}, wall);
    box({0, 1.5f, back}, {8, 3, 0.2f}, wall);
    box({0, 1.05f, 6.88f}, {1.65f, 2.1f, 0.12f}, metal);
    box({0, 2.3f, 6.78f}, {1.65f, 0.12f, 0.12f}, green, 0.9f);

    for (int i = 0; i < laserCountFor(level); ++i) {
        const auto& laser = lasers[i];
        box(laser.center, laser.size, {1.0f, 0.22f, 0.31f}, 1.0f);
        const glm::vec3 axis = laser.size.x > laser.size.y
                             ? glm::vec3(laser.size.x / 2 - 0.15f, 0, 0) : glm::vec3(0, 1.58f, 0);
        for (float side : {-1.0f, 1.0f}) {
            box(laser.center + side * axis, {0.22f, 0.23f, 0.23f}, red, 0.65f);
        }
    }
    box({0, climbHeight / 2, climbZ}, {8, climbHeight, climbHalfDepth * 2}, metal);
    for (float side : {-1.0f, 1.0f}) {
        const float face = climbZ + side * (climbHalfDepth + 0.03f);
        for (int rung = 1; rung <= 4; ++rung) {
            box({ladderX, rung * 0.25f, face}, {ladderWidth, 0.065f, 0.10f}, cyan, 0.55f);
        }
        box({0, climbHeight - 0.06f, face}, {8, 0.08f, 0.06f}, cyan, 0.65f);
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
