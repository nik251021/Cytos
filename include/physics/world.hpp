#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <bridge/renderBridge.hpp>
#include <resourceManager/worldLoader.hpp>
#include <resourceManager/genome_registry.hpp>
#include <resourceManager/cellConfigLoader.hpp>

#include <physics/systems/physicSystem.hpp>
#include <physics/systems/biologicalSystem.hpp>
#include <physics/systems/genomeSystem.hpp>

class world {
    friend class worldApi;
private:
    std::unordered_map<std::string, CellTemplate> m_cellTemplates;
    worldSettings curSettings;
    entt::registry m_registry;
    GenomeRegistry& m_genomeRegistry;
public:
    world(std::string worldName, GenomeRegistry& registry);
    void loadCellConfigs(const std::vector<std::string>& filePaths);
    
    entt::entity spawnCell(const std::string& type, glm::vec2 pos, glm::vec2 vel, glm::vec4 color);
    entt::entity spawnCellFromModule(const std::string& genomeName, int moduleIndex, glm::vec2 position);
    entt::entity makeAdhesin(entt::entity cell1, entt::entity cell2, float restLength, float maxLength, float strength);

    void update(float dt);
    void prepareRenderer(RenderBridge& rb);
};