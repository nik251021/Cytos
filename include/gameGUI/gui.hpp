#pragma once
#include "GLFW/glfw3.h"
#include "gameGUI/inspectgui.hpp"
#include "gameGUI/spawngui.hpp"
#include <rendering/camera.hpp>

#include <physics/worldApi.hpp>

class gameGUI {
private:
    uint32_t m_selectedEntity = (uint32_t)-1;
    inspectGUI inspectgui;
    spawnGUI spawngui;
public:
    void init(GLFWwindow* window);
    
    void update(worldApi& world, GLFWwindow* window, Camera& camera); 
    
    void onClose();

    void setSelectedEntity(uint32_t id) { m_selectedEntity = id; }
};