#include <gameGUI/spawngui.hpp>
#include <imgui.h>

void spawnGUI::update(worldApi& world, GLFWwindow* window, Camera& camera) {
    ImGui::Begin("Genome Spawner");

    ImGui::Checkbox("Spawn Mode (Right Click)", &m_isSpawnModeActive);
    ImGui::Separator();
    ImGui::Text("Available Genomes:");

    auto& genomeManager = world.getGenomeManager();
    std::vector<std::string> genomeNames = genomeManager.getGenomeNames();

    if (genomeNames.empty()) {
        ImGui::TextDisabled("No genomes saved.");
    } else {
        for (size_t i = 0; i < genomeNames.size(); ++i) {
            std::string label = genomeNames[i];
            bool isSelected = (m_selectedGenomeIndex == static_cast<int>(i));

            if (ImGui::Selectable(label.c_str(), isSelected)) {
                m_selectedGenomeIndex = static_cast<int>(i);
            }
        }
    }

    ImGui::End();

    ImGuiIO& io = ImGui::GetIO();
    if (m_isSpawnModeActive && m_selectedGenomeIndex >= 0 && m_selectedGenomeIndex < static_cast<int>(genomeNames.size())) {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && !io.WantCaptureMouse) {
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);

            int width, height;
            glfwGetWindowSize(window, &width, &height);
            float winWidth = static_cast<float>(width);
            float winHeight = static_cast<float>(height);

            glm::vec2 mouseScreenPos(
                (float)mouseX - winWidth * 0.5f,
                (winHeight - (float)mouseY) - winHeight * 0.5f
            );
            
            glm::vec2 mouseWorldPos = mouseScreenPos / camera.getZoom() + camera.getPosition();

            std::string selectedName = genomeNames[m_selectedGenomeIndex];

            const auto& genomeData = genomeManager.getGenome(selectedName);
            int startModuleIndex = genomeData.startModule;

            world.spawnCellFromModule(selectedName, startModuleIndex, mouseWorldPos.x, mouseWorldPos.y);
        }
    }
}