#include "physics/systems/biologicalSystem.hpp"
#include "physics/systems/genomeSystem.hpp"
#include <iostream>
#include <physics/world.hpp>
#include <physics/components.hpp>
#include <benchmark.hpp>

#include <cellsSystems/devorocyteLogic.hpp>
#include <cellsSystems/phagocyteLogic.hpp>

world::world(std::string name, resourceManager& resManager) 
    : m_resourceManager(resManager) {
    
    this->curSettings = m_resourceManager.getWorldManager().getWorldSettings(name);

    PhagocyteLogic::init(m_registry);
    DevorocyteLogic::init(m_registry);
}

entt::entity world::spawnCell(const std::string& type, glm::vec2 pos, glm::vec2 vel, glm::vec4 color) {
    const CellTemplate* t = m_resourceManager.getCellsConfigManager().getTemplate(type);
    if (!t) {
        std::cerr << "[World] Error: Cell template not found: " << type << std::endl;
        return entt::null;
    }
    
    auto entity = m_registry.create();
    m_registry.emplace<Position>(entity, pos);
    m_registry.emplace<Velocity>(entity, vel);
    m_registry.emplace<Mass>(entity, t->Default_mass);
    m_registry.emplace<Force>(entity, glm::vec2(0.0f));

    Methabolism met;
    met.atf = t->maxAtf;
    met.maxAtf = t->maxAtf;
    met.atfConsumptionRate = t->atfConsumptionRate;
    met.massConsumptionRate = t->massConsumptionRate;
    
    m_registry.emplace<Methabolism>(entity, met);
    m_registry.emplace<RenderData>(entity, color, t->maxRadius, (float)t->typeId);

    return entity;
}

entt::entity world::spawnCellFromModule(const std::string& genomeName, int moduleIndex, glm::vec2 position) {
    const auto& mod = m_resourceManager.getGenomeManager().getModule(genomeName, moduleIndex);
    
    std::string type = mod.cell_type; 
    
    const CellTemplate* t = m_resourceManager.getCellsConfigManager().getTemplate(type);
    if (!t) {
        std::cerr << "[World] Error: Cell type '" << type << "' not found for genome " << genomeName << "!" << std::endl;
        return entt::null;
    }

    bool shouldMakeAdhesin = false;
    if (mod.flags.count("makeAdhesin")) {
        shouldMakeAdhesin = mod.flags.at("makeAdhesin");
    }

    auto entity = m_registry.create();
    m_registry.emplace<GenomeComponent>(entity, genomeName, moduleIndex);
    m_registry.emplace<SplitComponent>(entity, mod.split_mass, shouldMakeAdhesin);
    m_registry.emplace<ReproductionCooldown>(entity, 0.4f, 0.4f);
    m_registry.emplace<Position>(entity, position);
    m_registry.emplace<Velocity>(entity, glm::vec2(0.0f));
    m_registry.emplace<Mass>(entity, t->Default_mass);
    m_registry.emplace<Force>(entity, glm::vec2(0.0f));

    float initialRotation = 0.0f;
    if (mod.params.count("flagellum_angle")) {
        initialRotation = mod.getParam("flagellum_angle", 0.0f);
    }
    m_registry.emplace<Rotation>(entity, initialRotation);

    if (mod.cell_type == "Flagellocyte") {
        float speed = mod.getParam("flagellum_speed", 10.0f);
        float consumption = mod.getParam("flagellum_consumption", 0.5f);
        m_registry.emplace<Flagellum>(entity, speed, consumption, true);
    }
    else if (mod.cell_type == "Devorocite") {
        m_registry.emplace<Devorocite>(entity);
    }
    else if (mod.cell_type == "Keratinocite") {
        m_registry.emplace<Keratinocite>(entity);
    }
    else if (mod.cell_type == "Axonocyte") {
        m_registry.emplace<Signals>(entity);
    }
    else if (mod.cell_type == "Neurocyte") {
        m_registry.emplace<Signals>(entity);
        
        NeuronComponent neuronComp;
        for (const auto& cData : mod.channels) {
            std::function<float(float)> formula;
            float arg = cData.argument;

            if (cData.formulaType == "multiply") {
                formula = [arg](float val) { return val * arg; };
            }
            else if (cData.formulaType == "add") {
                formula = [arg](float val) { return val + arg; };
            }
            else if (cData.formulaType == "divide") {
                formula = [arg](float val) { return (arg != 0.0f) ? (val / arg) : val; };
            }
            else if (cData.formulaType == "power") {
                formula = [arg](float val) { return std::pow(val, arg); };
            }
            else if (cData.formulaType == "threshold") {
                formula = [arg](float val) { return (val >= arg) ? val : 0.0f; };
            }
            else if (cData.formulaType == "clamp") {
                formula = [arg](float val) { return std::clamp(val, 0.0f, arg); };
            }
            else {
                formula = [](float val) { return val; };
            }

            neuronComp.channells.push_back({
                .inputNumber = cData.inputNumber,
                .outputNumber = cData.outputNumber,
                .formule = formula
            });
        }
        m_registry.emplace<NeuronComponent>(entity, neuronComp);
    }
    else if (mod.cell_type == "Sensorocyte") {
        SensorocyteComponent sensorComp;
        sensorComp.sensorType = static_cast<int>(mod.getParam("sensorType", 0.0f));
        sensorComp.outputNumber = static_cast<int>(mod.getParam("outputNumber", 0.0f));
        sensorComp.sensitivity = mod.getParam("sensitivity", 1.0f);

        m_registry.emplace<SensorocyteComponent>(entity, sensorComp);
        m_registry.emplace<Signals>(entity);
    }

    Methabolism met;
    met.atf = t->maxAtf; 
    met.maxAtf = t->maxAtf;
    met.atfConsumptionRate = t->atfConsumptionRate;
    met.massConsumptionRate = t->massConsumptionRate;
    m_registry.emplace<Methabolism>(entity, met);
    
    glm::vec4 color(
        mod.getParam("color_r", 1.0f),
        mod.getParam("color_g", 0.5f),
        mod.getParam("color_b", 0.0f),
        mod.getParam("color_a", 1.0f)
    );

    m_registry.emplace<RenderData>(entity, color, t->maxRadius, (float)t->typeId);

    return entity;
}

entt::entity world::makeAdhesin(entt::entity cell1, entt::entity cell2, float restLength, float maxLength, float strength) {
    if (!m_registry.valid(cell1) || !m_registry.valid(cell2)) return entt::null;
    if (cell1 == cell2) return entt::null;
    
    auto e = m_registry.create();
    m_registry.emplace<Adhesion>(e, cell1, cell2, restLength, maxLength, strength);

    return e;
}

entt::entity world::getCellAtPosition(glm::vec2 worldPos) {
    auto view = m_registry.view<Position, RenderData>();
    for (auto entity : view) {
        auto& pos = view.get<Position>(entity).value;
        auto& rd = view.get<RenderData>(entity);
        
        float dist = glm::distance(pos, worldPos);
        if (dist <= rd.radius) {
            return entity;
        }
    }
    return entt::null;
}

void world::update(float dt) {
    int entityCount = (int)m_registry.storage<Position>().size();

    DevorocyteLogic::setDeltaTime(dt);

    {
        ZONE_SCOPED("1. PhysicsSystem_Substep");
        PhysicsSystem::update(m_registry, curSettings, dt);
    }

    {
        ZONE_SCOPED("2. BiologicalSystem");
        BiologicalSystem::update(m_registry, dt, curSettings.light_amount);
    }

    {
        ZONE_SCOPED("3. GenomeSystem");
        GenomeSystem::update(*this, m_registry, m_resourceManager.getGenomeManager(), dt);
    }

    getBenchmark().tick(entityCount);
}

void world::prepareRenderer(RenderBridge& rb) {
    auto view = m_registry.view<Position, RenderData>();
    
    for (auto entity : view) {
        auto& pos = view.get<Position>(entity);
        auto& rd  = view.get<RenderData>(entity);
        
        rb.drawCell(pos.value, rd.radius, rd.type, rd.color);
    }
}

void world::applyDrag(entt::entity entity, glm::vec2 targetPos) {
    if (!m_registry.valid(entity)) return;

    if (m_registry.all_of<Position, Velocity>(entity)) {
        auto& pos = m_registry.get<Position>(entity).value;
        auto& vel = m_registry.get<Velocity>(entity).value;
        
        glm::vec2 direction = targetPos - pos;
        vel = direction * 15.0f;
    }
}