#include "rendering/renderer.hpp"
#include <glad/glad.h>

Renderer::Renderer() : m_maxInstances(10000) {
    m_shader = std::make_unique<Shader>("data/shaders/cell.vert", "data/shaders/cell.frag");

    float vertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f
    };
    unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

    m_vao = std::make_unique<VertexArray>();
    m_vbo = std::make_unique<VertexBuffer>(vertices, sizeof(vertices));
    m_ebo = std::make_unique<IndexBuffer>(indices, 6);

    m_vao->addBuffer(*m_vbo);
    m_vao->setIndexBuffer(*m_ebo);

    m_instanceBuffer = std::make_unique<InstanceBuffer>(nullptr, m_maxInstances * sizeof(InstanceData));
    m_vao->addInstanceBuffer(*m_instanceBuffer);
}

void Renderer::beginScene(const Camera& camera) {
    glClearColor(0.1f, 0.1f, 0.11f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_shader->use();
    m_shader->setMat4("u_ViewProj", camera.getViewProjection());
}

void Renderer::drawCells(const std::vector<InstanceData>& cells) {
    if (cells.empty()) return;

    if (cells.size() > m_maxInstances) {
        m_maxInstances = cells.size() * 2;
        m_instanceBuffer = std::make_unique<InstanceBuffer>(nullptr, m_maxInstances * sizeof(InstanceData));
        m_vao->addInstanceBuffer(*m_instanceBuffer);
    }

    m_instanceBuffer->updateData(cells.data(), cells.size() * sizeof(InstanceData));

    m_vao->bind();
    glDrawElementsInstanced(GL_TRIANGLES, m_ebo->getCount(), GL_UNSIGNED_INT, nullptr, cells.size());
}

void Renderer::drawRect(glm::vec2 pos, glm::vec2 size, glm::vec4 color) {
    InstanceData frame(pos + size * 0.5f, size.x * 0.5f, -99.0f, color);

    m_instanceBuffer->updateData(&frame, sizeof(InstanceData));
    
    m_vao->bind();
    glDrawArrays(GL_LINE_LOOP, 0, 4); 
}

void Renderer::endScene() {

}