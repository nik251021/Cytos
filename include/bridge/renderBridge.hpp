#pragma once
#include <vector>
#include "instance_data.hpp"
#include "../rendering/renderer.hpp"

class RenderBridge {
private:
    std::vector<InstanceData> m_Queue;
    Renderer& m_renderer;

public:
    RenderBridge(Renderer& renderer) : m_renderer(renderer) {}

    void drawCell(const glm::vec2& pos, float radius, float type, const glm::vec4& color) {
        m_Queue.emplace_back(pos, radius, type, color);
    }

    void drawRect(glm::vec2 pos, glm::vec2 size, glm::vec4 color) {
        m_Queue.emplace_back(pos + size * 0.5f, size.x * 0.5f, -1.0f, color);
    }

    const std::vector<InstanceData>& getQueue() const { return m_Queue; }
    void clear() { m_Queue.clear(); }
};