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
    std::vector<entt::entity> entitiesInCells;
    
    std::vector<uint32_t> cellIndices_;
    std::vector<uint32_t> counts_;

    SpatialGrid(const glm::vec3& worldSize, float cellSize = 50.f);

    void resize(const glm::vec3& worldSize, float newCellSize = -1);
    int index(int x, int y, int z = 0) const noexcept;
    int getCellId(glm::vec2 pos) const;
    void clear();

    void rebuild(entt::registry& registry);

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

class PhysicsSystem {
private:
    static SpatialGrid m_grid;

public:
    static void update(entt::registry& registry, const worldSettings& settings, float dt);

private:
    static void applyForces(entt::registry& registry, const worldSettings& settings, float dt);
    static void integratePosition(entt::registry& registry, const worldSettings& settings, float dt);
    static void resolveCollisions(entt::registry& registry, float dt);
};