#pragma once

class InstanceBuffer {
public:
    InstanceBuffer(const void* data, unsigned int size);
    ~InstanceBuffer();
    void bind() const;
    void updateData(const void* data, unsigned int size);
private:
    unsigned int m_rendererID;
};