#include "Shader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) { std::cerr << "Cannot open shader: " << path << "\n"; return ""; }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

unsigned int Shader::compile(unsigned int type, const std::string& path) {
    std::string src = readFile(path);
    const char* c = src.c_str();
    unsigned int s = glCreateShader(type);
    glShaderSource(s, 1, &c, nullptr);
    glCompileShader(s);
    int ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024]; glGetShaderInfoLog(s, 1024, nullptr, log);
        std::cerr << "Shader error (" << path << "):\n" << log << "\n";
    }
    return s;
}

Shader::Shader(const std::string& vertPath, const std::string& fragPath) {
    unsigned int vs = compile(GL_VERTEX_SHADER, vertPath);
    unsigned int fs = compile(GL_FRAGMENT_SHADER, fragPath);
    id = glCreateProgram();
    glAttachShader(id, vs);
    glAttachShader(id, fs);
    glLinkProgram(id);
    int ok; glGetProgramiv(id, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024]; glGetProgramInfoLog(id, 1024, nullptr, log);
        std::cerr << "Link error:\n" << log << "\n";
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
}

Shader::~Shader() { glDeleteProgram(id); }
void Shader::use() const { glUseProgram(id); }
void Shader::setInt(const std::string& n, int v) const { glUniform1i(glGetUniformLocation(id, n.c_str()), v); }
void Shader::setFloat(const std::string& n, float v) const { glUniform1f(glGetUniformLocation(id, n.c_str()), v); }
void Shader::setVec3(const std::string& n, const glm::vec3& v) const { glUniform3fv(glGetUniformLocation(id, n.c_str()), 1, glm::value_ptr(v)); }
void Shader::setMat4(const std::string& n, const glm::mat4& m) const { glUniformMatrix4fv(glGetUniformLocation(id, n.c_str()), 1, GL_FALSE, glm::value_ptr(m)); }
