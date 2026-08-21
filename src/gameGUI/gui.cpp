#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <imgui.h>

#include <gameGUI/gui.hpp>
#include <gameGUI/spawngui.hpp>

void gameGUI::init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void gameGUI::update(worldApi& world, GLFWwindow* window, Camera& camera) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("My Window");
    ImGui::Text("Hello from GameGUI!");
    ImGui::End();

    spawngui.update(world, window, camera);

    if (m_selectedEntity != (uint32_t)-1) {
        inspectgui.update(world, m_selectedEntity);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void gameGUI::onClose() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}