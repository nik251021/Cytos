#pragma once
#include <glm/glm.hpp>
#include <string>
#include <entt/entt.hpp>

#include <physics/world.hpp>
#include <bridge/renderBridge.hpp>
#include <physics/components.hpp>

#include <resourceManager/genome_registry.hpp>

class worldApi
{
private:
    world curWorld;
    GenomeRegistry m_registry;
public:
    worldApi() : m_registry(), curWorld("world1.json", m_registry) {
        m_registry.loadGenome("Genome1");
    }
    
    uint32_t spawnCell(
        std::string name,
        float posX, float posY, 
        float velocityX, float velocityY,
        glm::vec4 color
    ) {
        return (uint32_t)curWorld.spawnCell(
            name, 
            glm::vec2(posX, posY), 
            glm::vec2(velocityX, velocityY),
            color
        );
    }

    uint32_t spawnCellFromModule(std::string genomeName, int moduleIndex, float posX, float posY) {
        return (uint32_t)curWorld.spawnCellFromModule(
            genomeName, 
            moduleIndex, 
            glm::vec2(posX, posY)
        );
    }

    uint32_t createAdhesion(uint32_t id1, uint32_t id2, float rest, float max, float strength) {
        return (uint32_t)curWorld.makeAdhesin((entt::entity)id1, (entt::entity)id2, rest, max, strength);
    }

    void update(float dt, RenderBridge& rb) {
        curWorld.update(dt);
        curWorld.prepareRenderer(rb);
    }

    float getCellAtf(uint32_t id) {
        if (auto* met = curWorld.m_registry.try_get<Methabolism>((entt::entity)id)) {
            return met->atf;
        }
        return 0.0f;
    }
};