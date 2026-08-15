#pragma once
#include <glm/glm.hpp>

struct InstanceData {
    glm::vec2 position;
    float radius;
    float cellType;
    glm::vec4 color;

    InstanceData(glm::vec2 pos, float rad, float type, glm::vec4 col)
        : position(pos), radius(rad), cellType(type), color(col) {}
};