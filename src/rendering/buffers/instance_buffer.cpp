#include "rendering/buffers/instance_buffer.hpp"
#include <glad/glad.h>

InstanceBuffer::InstanceBuffer(const void* data, unsigned int size) {
    glGenBuffers(1, &m_rendererID);
    glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
}

InstanceBuffer::~InstanceBuffer() { glDeleteBuffers(1, &m_rendererID); }
void InstanceBuffer::bind() const { glBindBuffer(GL_ARRAY_BUFFER, m_rendererID); }

void InstanceBuffer::updateData(const void* data, unsigned int size) {
    bind();
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}