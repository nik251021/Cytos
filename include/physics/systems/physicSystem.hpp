#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <physics/components.hpp>
#include <resourceManager/worldLoader.hpp>
#include <unordered_map>
#include <vector>

struct SpatialGrid {
    static constexpr int CELL_SIZE = 50;
    std::unordered_map<int, std::vector<entt::entity>> cells;

    int getCellId(glm::vec2 pos) const {
        int x = static_cast<int>(pos.x / CELL_SIZE);
        int y = static_cast<int>(pos.y / CELL_SIZE);
        return x * 10000 + y;
    }
    
    void clear() { cells.clear(); }
    void add(entt::entity e, glm::vec2 pos) { cells[getCellId(pos)].push_back(e); }

    void getNeighboringCells(glm::vec2 pos, int* out) const {
        int cx = static_cast<int>(pos.x / CELL_SIZE);
        int cy = static_cast<int>(pos.y / CELL_SIZE);
        int i = 0;
        for(int x = cx - 1; x <= cx + 1; ++x) {
            for(int y = cy - 1; y <= cy + 1; ++y) {
                out[i++] = x * 10000 + y;
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