#pragma once
#include <vector>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

class Mesh {
public:
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    ~Mesh();
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& o) noexcept;
    Mesh& operator=(Mesh&& o) noexcept;

    void draw() const;

    static Mesh createCube();                                 // unit cube centered at origin
    static Mesh createPlane(float halfSize, float uvTiling);  // XZ plane, normal up

private:
    unsigned int vao = 0, vbo = 0, ebo = 0;
    int indexCount = 0;
    void release();
};
