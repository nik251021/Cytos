#pragma once
#include <vector>
#include <memory>
#include "rendering/camera.hpp"
#include "rendering/shader.hpp"
#include "rendering/buffers/VAO.hpp"
#include "rendering/buffers/VBO.hpp"
#include "rendering/buffers/EBO.hpp"
#include "rendering/buffers/instance_buffer.hpp"
#include "bridge/instance_data.hpp"

class Renderer {
public:
    Renderer();
    ~Renderer() = default;

    void beginScene(const Camera& camera);
    
    void drawCells(const std::vector<InstanceData>& cells);
    void drawRect(glm::vec2 pos, glm::vec2 size, glm::vec4 color);

    void endScene();

private:
    std::unique_ptr<Shader> m_shader;
    std::unique_ptr<VertexArray> m_vao;
    std::unique_ptr<VertexBuffer> m_vbo;
    std::unique_ptr<IndexBuffer> m_ebo;
    std::unique_ptr<InstanceBuffer> m_instanceBuffer;
    
    size_t m_maxInstances;
};