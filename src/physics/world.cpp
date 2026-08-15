#include "physics/systems/biologicalSystem.hpp"
#include "physics/systems/genomeSystem.hpp"
#include "physics/systems/physicSystem.hpp"
#include <iostream>
#include <physics/world.hpp>
#include <physics/components.hpp>

world::world(std::string name, GenomeRegistry& registry) : m_genomeRegistry(registry){
    this->curSettings = getWorldSettings(name);

    loadCellConfigs({ "data/configs/phagocyte.json", "data/configs/flagellocyte.json", "data/configs/photocyte.json"});
}

entt::entity world::spawnCell(const std::string& type, glm::vec2 pos, glm::vec2 vel, glm::vec4 color) {
    if (m_cellTemplates.find(type) == m_cellTemplates.end()) return entt::null;

    const auto& t = m_cellTemplates[type];
    
    auto entity = m_registry.create();
    m_registry.emplace<Position>(entity, pos);
    m_registry.emplace<Velocity>(entity, vel);
    m_registry.emplace<Mass>(entity, t.Default_mass);
    m_registry.emplace<Force>(entity, glm::vec2(0.0f));

    Methabolism met;
    met.atf = t.maxAtf;
    met.maxAtf = t.maxAtf;
    met.atfConsumptionRate = t.atfConsumptionRate;
    met.massConsumptionRate = t.massConsumptionRate;
    
    m_registry.emplace<Methabolism>(entity, met);
    m_registry.emplace<RenderData>(entity, color, t.maxRadius, t.typeId);

    return entity;
}

entt::entity world::spawnCellFromModule(const std::string& genomeName, int moduleIndex, glm::vec2 position) {
    const auto& mod = m_genomeRegistry.getModule(genomeName, moduleIndex);
    
    std::string type = mod.cell_type; 
    
    if (m_cellTemplates.find(type) == m_cellTemplates.end()) {
        std::cerr << "Error: Cell type '" << type << "' not found!" << std::endl;
        return entt::null;
    }
    bool shouldMakeAdhesin = false;
    if (mod.flags.count("makeAdhesin")) {
        shouldMakeAdhesin = mod.flags.at("makeAdhesin");
    }
    
    const auto& t = m_cellTemplates.at(type); 

    auto entity = m_registry.create();
    m_registry.emplace<GenomeComponent>(entity, genomeName, moduleIndex);
    m_registry.emplace<SplitComponent>(entity, mod.split_mass, shouldMakeAdhesin);
    m_registry.emplace<ReproductionCooldown>(entity, 0.4f, 0.4f);
    m_registry.emplace<Position>(entity, position);
    m_registry.emplace<Velocity>(entity, glm::vec2(0.0f));
    m_registry.emplace<Mass>(entity, t.Default_mass);
    m_registry.emplace<Force>(entity, glm::vec2(0.0f));

    float initialRotation = 0.0f;

    if (mod.params.count("flagellum_angle")) {
        initialRotation = mod.getParam("flagellum_angle", 0.0f);
    }
    m_registry.emplace<Rotation>(entity, initialRotation);

    if (mod.cell_type == "Flagellocyte") {
        float speed = mod.getParam("flagellum_speed", 10);
        float consumption = mod.getParam("flagellum_consumption", 0.5f);
        m_registry.emplace<Flagellum>(entity, speed, consumption, true);
    }

    Methabolism met;
    met.atf = t.maxAtf; 
    met.maxAtf = t.maxAtf;
    met.atfConsumptionRate = t.atfConsumptionRate;
    met.massConsumptionRate = t.massConsumptionRate;
    m_registry.emplace<Methabolism>(entity, met);
    
    auto getColor = [&](const std::string& key, float defaultVal) {
        auto it = mod.params.find(key);
        return (it != mod.params.end()) ? it->second : defaultVal;
    };

    glm::vec4 color(
        getColor("color_r", 1.0f),
        getColor("color_g", 0.5f),
        getColor("color_b", 0.0f),
        getColor("color_a", 1.0f)
    );

    m_registry.emplace<RenderData>(entity, color, t.maxRadius, (float)t.typeId);

    return entity;
}

entt::entity world::makeAdhesin(entt::entity cell1, entt::entity cell2, float restLength, float maxLength, float strength) {
    if (!m_registry.valid(cell1) || !m_registry.valid(cell2)) return entt::null;

    if (cell1 == cell2) return entt::null;
    
    auto e = m_registry.create();
    m_registry.emplace<Adhesion>(e, cell1, cell2, restLength, maxLength, strength);

    return e;
}
void onCollision(entt::entity e1, entt::entity e2) {

}



void world::update(float dt) {
    PhysicsSystem::update(m_registry, curSettings, dt);
    BiologicalSystem::update(m_registry, dt);
    GenomeSystem::update(*this, m_registry, m_genomeRegistry, dt);
}

void world::prepareRenderer(RenderBridge& rb) {
    auto view = m_registry.view<Position, RenderData>();
    
    for (auto entity : view) {
        auto& pos = view.get<Position>(entity);
        auto& rd  = view.get<RenderData>(entity);
        
        rb.drawCell(pos.value, rd.radius, rd.type, rd.color);
    }
}

void world::loadCellConfigs(const std::vector<std::string>& filePaths) {
    for (const auto& path : filePaths) {
        CellTemplate t = loadCellTemplate(path);
        m_cellTemplates[t.displayName] = t;
    }
}