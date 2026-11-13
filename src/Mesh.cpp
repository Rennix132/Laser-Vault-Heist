#include "Mesh.h"
#include <glad/glad.h>
#include <cstddef>
#include <utility>

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    indexCount = static_cast<int>(indices.size());
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glBindVertexArray(0);
}

void Mesh::release() {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (ebo) glDeleteBuffers(1, &ebo);
    vao = vbo = ebo = 0;
}

Mesh::~Mesh() { release(); }

// Move support is needed because the static factories return Mesh by value.
Mesh::Mesh(Mesh&& o) noexcept : vao(o.vao), vbo(o.vbo), ebo(o.ebo), indexCount(o.indexCount) {
    o.vao = o.vbo = o.ebo = 0;
    o.indexCount = 0;
}

Mesh& Mesh::operator=(Mesh&& o) noexcept {
    if (this != &o) {
        release();
        vao = o.vao; vbo = o.vbo; ebo = o.ebo; indexCount = o.indexCount;
        o.vao = o.vbo = o.ebo = 0;
        o.indexCount = 0;
    }
    return *this;
}

void Mesh::draw() const {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

Mesh Mesh::createCube() {
    std::vector<Vertex> v;
    std::vector<unsigned int> idx;
    // face normal n, and two tangent axes u, w (chosen so that u x w = n -> CCW winding)
    const glm::vec3 n[6] = {{0,0,1},{0,0,-1},{1,0,0},{-1,0,0},{0,1,0},{0,-1,0}};
    const glm::vec3 u[6] = {{1,0,0},{-1,0,0},{0,0,-1},{0,0,1},{1,0,0},{1,0,0}};
    const glm::vec3 w[6] = {{0,1,0},{0,1,0},{0,1,0},{0,1,0},{0,0,-1},{0,0,1}};
    const float su[4] = {-1, 1, 1, -1};
    const float sw[4] = {-1, -1, 1, 1};
    const glm::vec2 uvs[4] = {{0,0},{1,0},{1,1},{0,1}};

    for (int f = 0; f < 6; f++) {
        unsigned int base = static_cast<unsigned int>(v.size());
        for (int i = 0; i < 4; i++) {
            glm::vec3 p = n[f] * 0.5f + u[f] * (su[i] * 0.5f) + w[f] * (sw[i] * 0.5f);
            v.push_back({p, n[f], uvs[i]});
        }
        idx.insert(idx.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
    }
    return Mesh(v, idx);
}

Mesh Mesh::createPlane(float s, float t) {
    std::vector<Vertex> v = {
        {{-s, 0,  s}, {0,1,0}, {0, 0}},
        {{ s, 0,  s}, {0,1,0}, {t, 0}},
        {{ s, 0, -s}, {0,1,0}, {t, t}},
        {{-s, 0, -s}, {0,1,0}, {0, t}},
    };
    std::vector<unsigned int> idx = {0, 1, 2, 0, 2, 3};
    return Mesh(v, idx);
}
