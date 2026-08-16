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

void handleCameraInput(Camera& camera, GLFWwindow* window, float dt) {
    float speed = 500.0f / camera.getZoom(); 
    glm::vec2 pos = camera.getPosition();

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)  pos.x -= speed * dt;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)  pos.x += speed * dt;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)  pos.y += speed * dt;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)  pos.y -= speed * dt;

    camera.setPosition(pos);
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

    uint32_t cellFromGenome = world.spawnCellFromModule("Genome1", 1, 450.0f, 500.0f);

    uint32_t projectile = world.spawnCell("Phagocyte", 450, 10, 0.1, 10000, glm::vec4(1,1,1,0));
    
    while (!window.shouldClose()) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        
        renderBridge.clear();

        world.update(deltaTime, renderBridge);

        renderer.beginScene(camera);
        renderer.drawRect(glm::vec2(0, 0), glm::vec2(1000, 1000), glm::vec4(1.0f));
        renderer.drawCells(renderBridge.getQueue());
        renderer.endScene();

        handleCameraInput(camera, window.getNativeWindow(), deltaTime);

        window.swapBuffers();
        window.pollEvents();
    }

    return 0;
}