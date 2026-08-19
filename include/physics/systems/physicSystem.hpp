#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <physics/components.hpp>
#include <resourceManager/worldLoader.hpp>
#include <vector>
#include <span>

struct SpatialGrid {
    glm::uvec3 size{0u};
    float cellSize;
    size_t countCells;

    std::vector<uint32_t> offsets;
    std::vector<uint32_t> cellIndices_;
    std::vector<uint32_t> counts_;
    std::vector<entt::entity> entitiesInCells;

    SpatialGrid(const glm::vec3& worldSize, float cellSize = 50.f);

    void resize(const glm::vec3& worldSize, float newCellSize = -1);
    int index(int x, int y, int z = 0) const noexcept;
    int getCellId(glm::vec2 pos) const;
    void clear();

    void rebuild(const std::vector<glm::vec2>& positions, const std::vector<entt::entity>& entities);

    std::span<const entt::entity> entitiesInCell(int x, int y) const noexcept;

    template <typename Func>
    void forEachNeighborEntity(glm::vec2 pos, Func&& func) const {
        int cx = static_cast<int>(pos.x / cellSize) + 1;
        int cy = static_cast<int>(pos.y / cellSize) + 1;

        for (int x = cx - 1; x <= cx + 1; ++x) {
            for (int y = cy - 1; y <= cy + 1; ++y) {
                if (x < 0 || x >= static_cast<int>(size.x) || y < 0 || y >= static_cast<int>(size.y)) continue;
                
                auto span = entitiesInCell(x, y);
                for (auto entityB : span) {
                    func(entityB);
                }
            }
        }
    }
};

struct WorldSOA {
    std::vector<entt::entity> entities;
    std::vector<glm::vec2> pos;
    std::vector<glm::vec2> vel;
    std::vector<glm::vec2> force;
    std::vector<float> mass;
    std::vector<float> radius;
    std::vector<float> renderType;
    
    std::vector<size_t> entityToIndex;

    void resize(size_t capacity) {
        entities.resize(capacity);
        pos.resize(capacity);
        vel.resize(capacity);
        force.resize(capacity);
        mass.resize(capacity);
        radius.resize(capacity);
        renderType.resize(capacity);
    }

    void clear() {
        entities.clear();
        pos.clear();
        vel.clear();
        force.clear();
        mass.clear();
        radius.clear();
        renderType.clear();
    }
};

class PhysicsSystem {
private:
    static SpatialGrid m_grid;
    static WorldSOA m_soa;

public:
    static void update(entt::registry& registry, const worldSettings& settings, float dt);
private:
    static void pullFromRegistry(entt::registry& registry);
    static void pushToRegistry(entt::registry& registry);

    static void applyForces(const worldSettings& settings, float dt);
    static void integratePosition(const worldSettings& settings, float dt);
    static void resolveCollisions(entt::registry& registry, float dt);

    static bool tryPhagocyteConsumption(entt::registry& registry, entt::entity eater, entt::entity victim);
    static bool tryDevourEnergy(entt::registry& registry, entt::entity eater, entt::entity victim, float dt);
};