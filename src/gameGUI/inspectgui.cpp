#include "imgui.h"
#include <gameGUI/inspectgui.hpp>

#include <gameGUI/inspectgui.hpp>
#include <imgui.h>
#include <physics/worldApi.hpp>

void inspectGUI::update(worldApi& world, uint32_t entityID) {
    if (entityID == (uint32_t)entt::null) return;

    ImGui::Begin("Cell Inspector");

    ImGui::Text("Entity ID: %u", entityID);
    ImGui::Separator();

    if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (auto* pos = world.getCellPosition(entityID)) {
            ImGui::Text("Position: X: %.2f, Y: %.2f", pos->value.x, pos->value.y);
        }
        if (auto* vel = world.getCellVelocity(entityID)) {
            ImGui::Text("Velocity: X: %.2f, Y: %.2f", vel->value.x, vel->value.y);
        }
        if (auto* mass = world.getCellMass(entityID)) {
            ImGui::DragFloat("Mass", &mass->value, 1.0f, 0.1f, 1000.0f);
        }
    }

    if (ImGui::CollapsingHeader("Metabolism")) {
        if (auto* metab = world.getCellMetabolism(entityID)) {
            ImGui::Text("ATF: %.1f / %.1f", metab->atf, metab->maxAtf);
            float percent = metab->maxAtf > 0 ? metab->atf / metab->maxAtf : 0.0f;
            ImGui::ProgressBar(percent, ImVec2(0.0f, 0.0f), "ATF Level");
            ImGui::Checkbox("Is Active", &metab->isActive);
        }
    }

    if (ImGui::CollapsingHeader("Genome")) {
        if (auto* genome = world.getCellGenome(entityID)) {
            ImGui::Text("Name: %s", genome->genomeName.c_str());
            ImGui::Text("Module Index: %d", genome->currentModuleIndex);
        }
    }

    if (auto* flagellum = world.getCellFlagellum(entityID)) {
        if (ImGui::CollapsingHeader("Flagellum")) {
            ImGui::Text("Speed: %.2f", flagellum->speed);
            ImGui::Checkbox("Enabled", &flagellum->enabled);
        }
    }

    ImGui::End();
}