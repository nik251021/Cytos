#pragma once
#include "GLFW/glfw3.h"
#include "gameGUI/inspectgui.hpp"

#include <physics/worldApi.hpp>

class gameGUI {
private:
    uint32_t m_selectedEntity = (uint32_t)-1;
    inspectGUI inspectgui;
public:
    void init(GLFWwindow* window);
    void update(worldApi& world);
    void onClose();

    void setSelectedEntity(uint32_t id) { m_selectedEntity = id; }
};