#include "rendering/shader.hpp"
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    } catch (std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
    }

    unsigned int vertex = compileShader(GL_VERTEX_SHADER, vertexCode);
    unsigned int fragment = compileShader(GL_FRAGMENT_SHADER, fragmentCode);

    m_rendererID = glCreateProgram();
    glAttachShader(m_rendererID, vertex);
    glAttachShader(m_rendererID, fragment);
    glLinkProgram(m_rendererID);

    int success;
    glGetProgramiv(m_rendererID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_rendererID, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::~Shader() {
    glDeleteProgram(m_rendererID);
}

unsigned int Shader::compileShader(unsigned int type, const std::string& source) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(id, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILE_FAILED: " << infoLog << std::endl;
    }
    return id;
}

void Shader::use() const {
    glUseProgram(m_rendererID);
}

void Shader::setMat4(const std::string& name, const glm::mat4& matrix) const {
    glUniformMatrix4fv(glGetUniformLocation(m_rendererID, name.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::setVec2(const std::string& name, const glm::vec2& vec) const {
    glUniform2fv(glGetUniformLocation(m_rendererID, name.c_str()), 1, glm::value_ptr(vec));
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(m_rendererID, name.c_str()), value);
}