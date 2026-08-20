#include <cstdint>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "glm/core/type.hpp"
#include "rendering/window.hpp"
#include "rendering/camera.hpp"
#include "rendering/renderer.hpp"

#include "physics/worldApi.hpp"
#include "bridge/renderBridge.hpp"

#include <gameGUI/gui.hpp>

void handleCameraInput(Camera& camera, GLFWwindow* window, float dt) {
    float speed = 500.0f / camera.getZoom(); 
    glm::vec2 pos = camera.getPosition();

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)  pos.x -= speed * dt;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)  pos.x += speed * dt;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)  pos.y += speed * dt;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)  pos.y -= speed * dt;

    camera.setPosition(pos);
}

void handleControls(Window* window, Camera& camera, worldApi& world, gameGUI& gui) {
    if (window->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
        double mouseX, mouseY;
        glfwGetCursorPos(window->getNativeWindow(), &mouseX, &mouseY);

        float winWidth = (float)window->getWidth();
        float winHeight = (float)window->getHeight();

        glm::vec2 mouseScreenPos(
            (float)mouseX - winWidth * 0.5f,
            (winHeight - (float)mouseY) - winHeight * 0.5f
        );
        glm::vec2 mouseWorldPos = mouseScreenPos / camera.getZoom() + camera.getPosition();

        world.handleMouseInput(true, mouseWorldPos);
    } else {
        world.handleMouseInput(false, glm::vec2(0.0f));
    }

    if (window->isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        double mouseX, mouseY;
        glfwGetCursorPos(window->getNativeWindow(), &mouseX, &mouseY);

        float winWidth = (float)window->getWidth();
        float winHeight = (float)window->getHeight();

        glm::vec2 mouseScreenPos(
            (float)mouseX - winWidth * 0.5f,
            (winHeight - (float)mouseY) - winHeight * 0.5f
        );
        glm::vec2 mouseWorldPos = mouseScreenPos / camera.getZoom() + camera.getPosition();

        uint32_t clicked = world.getCellAtPosition(mouseWorldPos);
        gui.setSelectedEntity(clicked);
    }
}
static Camera* g_camera = nullptr;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (!g_camera) return;

    float zoom = g_camera->getZoom();
    
    if (yoffset > 0) {
        zoom *= 1.1f;
    } else if (yoffset < 0) {
        zoom /= 1.1f;
    }

    zoom = glm::clamp(zoom, 0.1f, 10.0f);
    g_camera->setZoom(zoom);
}

int main() {
    float lastTime = glfwGetTime();

    Window window(1000, 1000, "Cell Lab");
    Camera camera(1000.0f, 1000.0f);

    g_camera = &camera;
    glfwSetScrollCallback(window.getNativeWindow(), scroll_callback);

    Renderer renderer;
    RenderBridge renderBridge(renderer);
    worldApi world;

    gameGUI gui;
    gui.init(window.getNativeWindow());

    uint32_t cellFromGenome = world.spawnCellFromModule("Genome1", 1, 450.0f, 500.0f);
    uint32_t devorociteGenome = world.spawnCellFromModule("Devorocite", 1, 425, 500);
    uint32_t keratinociteGenome = world.spawnCellFromModule("Keratinocite", 1, 410, 500);
    uint32_t neurocyteGenome = world.spawnCellFromModule("Neurocyte", 1, 400, 500);
    
    uint32_t projectile = world.spawnCell("Phagocyte", 450, 10, 0.1, 10000, glm::vec4(1,1,1,0));
    
    while (!window.shouldClose()) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        
        renderBridge.clear();

        handleControls(&window, camera, world, gui);

        world.update(deltaTime, renderBridge);

        renderer.beginScene(camera);
        renderer.drawRect(glm::vec2(0, 0), glm::vec2(1000, 1000), glm::vec4(1.0f));
        renderer.drawCells(renderBridge.getQueue());
        renderer.endScene();

        handleCameraInput(camera, window.getNativeWindow(), deltaTime);

        gui.update(world);

        window.swapBuffers();
        window.pollEvents();
    }

    gui.onClose();

    return 0;
}