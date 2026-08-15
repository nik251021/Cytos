#pragma once
#include <cstddef>

#include "VBO.hpp"
#include "EBO.hpp"
#include "instance_buffer.hpp"

#include "bridge/instance_data.hpp"

class VertexArray {
public:
    VertexArray();
    ~VertexArray();
    void bind() const;
    void unbind() const;
    void addBuffer(const VertexBuffer& vb);
    void setIndexBuffer(const IndexBuffer& ib);
    void addInstanceBuffer(const InstanceBuffer& ib);
private:
    unsigned int m_rendererID;
};