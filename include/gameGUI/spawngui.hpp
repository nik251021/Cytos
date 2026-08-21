#pragma once
#include <physics/worldApi.hpp>
#include <rendering/camera.hpp>
#include <GLFW/glfw3.h>

class spawnGUI {
private:
    bool m_isSpawnModeActive = false;
    int m_selectedGenomeIndex = -1;

public:
    void update(worldApi& world, GLFWwindow* window, Camera& camera);

    bool isSpawnModeActive() const { return m_isSpawnModeActive; }
};