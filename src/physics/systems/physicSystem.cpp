#include "benchmark.hpp"
#include <physics/systems/physicSystem.hpp>
#include <cmath>

SpatialGrid PhysicsSystem::m_grid(glm::vec3(4000.0f, 4000.0f, 1.0f), 50.0f);

SpatialGrid::SpatialGrid(const glm::vec3& worldSize, float cellSize) : cellSize(cellSize) {
    resize(worldSize, cellSize);
}

void SpatialGrid::resize(const glm::vec3& worldSize, float newCellSize) {
    if (newCellSize > 0) cellSize = newCellSize;
    size.x = static_cast<uint32_t>(worldSize.x / cellSize) + 2;
    size.y = static_cast<uint32_t>(worldSize.y / cellSize) + 2;
    size.z = 1;
    countCells = size.x * size.y * size.z;

    offsets.resize(countCells + 1);
    counts_.resize(countCells, 0);
}

int SpatialGrid::index(int x, int y, int z) const noexcept {
    return (z * size.y + y) * size.x + x;
}

int SpatialGrid::getCellId(glm::vec2 pos) const {
    int x = static_cast<int>(pos.x / cellSize) + 1;
    int y = static_cast<int>(pos.y / cellSize) + 1;
    x = std::clamp(x, 0, static_cast<int>(size.x) - 1);
    y = std::clamp(y, 0, static_cast<int>(size.y) - 1);
    return index(x, y, 0);
}

void SpatialGrid::clear() {}

void SpatialGrid::rebuild(entt::registry& registry) {
    auto view = registry.view<Position>();
    size_t numEntities = view.size();

    cellIndices_.resize(numEntities);
    entitiesInCells.resize(numEntities);
    std::fill(counts_.begin(), counts_.end(), 0);

    size_t idx = 0;
    view.each([&](auto entity, auto& pos) {
        int cellId = getCellId(pos.value);
        cellIndices_[idx++] = cellId;
        counts_[cellId]++;
    });

    offsets[0] = 0;
    for (size_t i = 0; i < countCells; ++i) {
        offsets[i + 1] = offsets[i] + counts_[i];
    }

    std::fill(counts_.begin(), counts_.end(), 0);
    idx = 0;
    view.each([&](auto entity, auto& pos) {
        int cellId = cellIndices_[idx++];
        uint32_t destIdx = offsets[cellId] + counts_[cellId]++;
        entitiesInCells[destIdx] = entity;
    });
}

std::span<const entt::entity> SpatialGrid::entitiesInCell(int x, int y) const noexcept {
    int cellId = index(x, y, 0);
    size_t begin = offsets[cellId];
    size_t end = offsets[cellId + 1];
    return std::span<const entt::entity>(entitiesInCells.data() + begin, end - begin);
}

bool tryPhagocyteConsumption(entt::registry& registry, entt::entity eater, entt::entity victim) {
    if (!registry.valid(eater) || !registry.valid(victim)) return false;

    auto* rdEater = registry.try_get<RenderData>(eater);
    auto* rdVictim = registry.try_get<RenderData>(victim);
    if (!rdEater || !rdVictim) return false;

    bool isPhagocyte = (rdEater->type == 0.0f && registry.all_of<Methabolism>(eater));
    bool isDeadCell = (rdVictim->type == 99.0f);

    if (isPhagocyte && isDeadCell) {
        auto& metEater = registry.get<Methabolism>(eater);
        auto& massEater = registry.get<Mass>(eater);
        auto& massVictim = registry.get<Mass>(victim);
        
        massEater.value += 0.25f + massVictim.value * 1.0f;
        metEater.atf = std::min(metEater.maxAtf, metEater.atf + 50.0f);

        if (massEater.value > 15.0f) {
            massEater.value = 15.0f;
        }

        registry.destroy(victim);
        return true;
    }

    return false;
}

void PhysicsSystem::update(entt::registry& registry, const worldSettings& settings, float dt) {
    ZONE_SCOPED("1. PhysicsSystem (Total)"); // Замеряет общую работу системы

    {
        ZONE_SCOPED("1.1. applyForces");
        applyForces(registry, settings, dt);
    }

    {
        ZONE_SCOPED("1.2. integratePosition");
        integratePosition(registry, settings, dt);
    }

    {
        ZONE_SCOPED("1.3. resolveCollisions");
        resolveCollisions(registry, dt);
    }
}

void PhysicsSystem::applyForces(entt::registry& registry, const worldSettings& settings, float dt) {
    registry.view<Velocity, Mass, Force>().each([&](auto& vel, auto& mass, auto& force) {
        glm::vec2 acceleration = (force.value + glm::vec2(0.0f, settings.gravity * mass.value) - (vel.value * settings.viscosity)) / mass.value;
        vel.value += acceleration * dt;
        vel.value *= glm::clamp(1.0f - settings.friction * dt, 0.0f, 1.0f);
        force.value = glm::vec2(0.0f); 
    });
}

void PhysicsSystem::integratePosition(entt::registry& registry, const worldSettings& settings, float dt) {
    registry.view<Position, Velocity>().each([&](auto& pos, auto& vel) {
        pos.value += vel.value * dt;
        const float bounce = 0.5f;
        if (pos.value.x < 0) { pos.value.x = 0; vel.value.x *= -bounce; }
        else if (pos.value.x > settings.sizeX) { pos.value.x = settings.sizeX; vel.value.x *= -bounce; }
        if (pos.value.y < 0) { pos.value.y = 0; vel.value.y *= -bounce; }
        else if (pos.value.y > settings.sizeY) { pos.value.y = settings.sizeY; vel.value.y *= -bounce; }
    });

    registry.view<Position, Rotation, Adhesion>().each([&](auto entity, auto& pos, auto& rot, auto& adj) {
        entt::entity partner = (adj.cellA == entity) ? adj.cellB : adj.cellA;
        
        if (registry.valid(partner) && registry.all_of<Position>(partner)) {
            glm::vec2 partnerPos = registry.get<Position>(partner).value;
            glm::vec2 diff = pos.value - partnerPos;
            
            if (glm::dot(diff, diff) > 0.0001f) {
                rot.angle = std::atan2(diff.y, diff.x);
            }
        }
    });
}

void PhysicsSystem::resolveCollisions(entt::registry& registry, float dt) {
    const int collisionIterations = 3;
    
    for (int iter = 0; iter < collisionIterations; ++iter) {
        // 1. Перестраиваем сетку КАЖДУЮ итерацию, чтобы знать актуальные позиции
        m_grid.rebuild(registry);

        auto view = registry.view<Position, Velocity, Mass, RenderData>();

        view.each([&](auto entityA, auto& posA, auto& velA, auto& massA, auto& rdA) {
            m_grid.forEachNeighborEntity(posA.value, [&](entt::entity entityB) {
                if (entityA >= entityB) return;
                
                // Проверка на случай, если entityB была уничтожена в этот же кадр
                if (!registry.valid(entityB)) return;

                auto& posB = registry.get<Position>(entityB);
                auto& rdB = registry.get<RenderData>(entityB);

                float minDist = rdA.radius + rdB.radius;
                float dx = posA.value.x - posB.value.x;
                if (std::abs(dx) > minDist) return;
                float dy = posA.value.y - posB.value.y;
                if (std::abs(dy) > minDist) return;

                float distSq = dx * dx + dy * dy;
                if (distSq < minDist * minDist && distSq > 0.000001f) {
                    // Сначала проверяем поедание
                    if (tryPhagocyteConsumption(registry, entityA, entityB)) return;
                    if (tryPhagocyteConsumption(registry, entityB, entityA)) return;

                    // Если не съели, разрешаем коллизию
                    auto& velB = registry.get<Velocity>(entityB);
                    auto& massB = registry.get<Mass>(entityB);

                    float dist = std::sqrt(distSq);
                    glm::vec2 normal = glm::vec2(dx, dy) / dist;
                    float overlap = minDist - dist;

                    float totalMass = massA.value + massB.value;
                    float m1Ratio = massB.value / totalMass;
                    float m2Ratio = massA.value / totalMass;

                    // Позиционная коррекция (меньше итераций = меньше фактор)
                    float pushFactor = 0.4f; 
                    posA.value += normal * (overlap * pushFactor) * m1Ratio;
                    posB.value -= normal * (overlap * pushFactor) * m2Ratio;

                    // Импульсы (упрощенно)
                    glm::vec2 relativeVel = velA.value - velB.value;
                    float velAlongNormal = glm::dot(relativeVel, normal);
                    
                    if (velAlongNormal < 0) {
                        float restitution = 0.2f;
                        float j = -(1.0f + restitution) * velAlongNormal;
                        j /= (1.0f / massA.value + 1.0f / massB.value);
                        
                        glm::vec2 impulse = j * normal;
                        velA.value += (1.0f / massA.value) * impulse;
                        velB.value -= (1.0f / massB.value) * impulse;
                    }
                }
            });
        });
    }
}