#pragma once
#include <string>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <bridge/renderBridge.hpp>
#include <resourceManager/resourceManager.hpp>

#include <physics/systems/physicSystem.hpp>
#include <physics/systems/biologicalSystem.hpp>
#include <physics/systems/genomeSystem.hpp>

class world {
    friend class worldApi;
private:
    resourceManager& m_resourceManager;
    WorldSettings curSettings;
    entt::registry m_registry;

public:
    world(std::string worldName, resourceManager& resManager);
    
    entt::entity spawnCell(const std::string& type, glm::vec2 pos, glm::vec2 vel, glm::vec4 color);
    entt::entity spawnCellFromModule(const std::string& genomeName, int moduleIndex, glm::vec2 position);
    entt::entity makeAdhesin(entt::entity cell1, entt::entity cell2, float restLength, float maxLength, float strength);

    entt::entity getCellAtPosition(glm::vec2 worldPos);

    void update(float dt);
    void prepareRenderer(RenderBridge& rb);

    void applyDrag(entt::entity entity, glm::vec2 targetPos);
};