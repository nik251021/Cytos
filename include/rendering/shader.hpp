#pragma once
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();
    void use() const;
    void setMat4(const std::string& name, const glm::mat4& matrix) const;
    void setVec2(const std::string& name, const glm::vec2& vec) const;
    void setFloat(const std::string& name, float value) const;

private:
    unsigned int m_rendererID;
    unsigned int compileShader(unsigned int type, const std::string& source);
};