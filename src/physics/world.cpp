#include <filesystem>
namespace fs = std::filesystem;

#include "physics/systems/biologicalSystem.hpp"
#include "physics/systems/genomeSystem.hpp"
#include "physics/systems/physicSystem.hpp"
#include <iostream>
#include <physics/world.hpp>
#include <physics/components.hpp>

#include <benchmark.hpp>

world::world(std::string name, GenomeRegistry& registry) : m_genomeRegistry(registry){
    this->curSettings = getWorldSettings(name);

    std::string configsDir = "data/configs";
    if (fs::exists(configsDir) && fs::is_directory(configsDir)) {
        for (const auto& entry : fs::directory_iterator(configsDir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                std::string pathStr = entry.path().string();
                CellTemplate t = loadCellTemplate(pathStr);
                m_cellTemplates[t.displayName] = t;
                std::cout << "[World] Auto-loaded cell config: " << t.displayName << std::endl;
            }
        }
    } else {
        std::cerr << "[World] Warning: Configs directory not found: " << configsDir << std::endl;
    }
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

    if (mod.cell_type == "Devorocite") {
        m_registry.emplace<Devorocite>(entity);
    }

    if (mod.cell_type == "Keratinocite") {
        m_registry.emplace<Keratinocite>(entity);
    }

    if (mod.cell_type == "Axonocyte") {
        m_registry.emplace<Signals>(entity);
    }

    if (mod.cell_type == "Neurocyte") {
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

    if (mod.cell_type == "Sensorocyte") {
        SensorocyteComponent sensorComp;
        
        std::cout << "[Spawn] Creating Sensorocyte. Params count: " << mod.params.size() << std::endl;
        for (const auto& [key, val] : mod.params) {
            std::cout << "  Param -> " << key << ": " << val << std::endl;
        }

        if (mod.params.count("sensorType")) {
            sensorComp.sensorType = static_cast<int>(mod.getParam("sensorType", 0.0f));
        }
        if (mod.params.count("outputNumber")) {
            sensorComp.outputNumber = static_cast<int>(mod.getParam("outputNumber", 0.0f));
        }
        if (mod.params.count("sensitivity")) {
            sensorComp.sensitivity = mod.getParam("sensitivity", 1.0f);
        }

        std::cout << "  -> Final Sensor: type=" << sensorComp.sensorType 
                  << ", channel=" << sensorComp.outputNumber << std::endl;

        m_registry.emplace<SensorocyteComponent>(entity, sensorComp);
        m_registry.emplace<Signals>(entity);
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

    {
        ZONE_SCOPED("1. PhysicsSystem_Substep");
        PhysicsSystem::update(m_registry, curSettings, dt);
    }

    {
        ZONE_SCOPED("2. BiologicalSystem");
        BiologicalSystem::update(m_registry, dt);
    }

    {
        ZONE_SCOPED("3. GenomeSystem");
        GenomeSystem::update(*this, m_registry, m_genomeRegistry, dt);
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