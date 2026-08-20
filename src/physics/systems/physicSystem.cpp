#include "benchmark.hpp"
#include <physics/systems/physicSystem.hpp>
#include <cmath>
#include <algorithm>

const int subSteps = 2;

SpatialGrid PhysicsSystem::m_grid(glm::vec3(4000.0f, 4000.0f, 1.0f), 12.0f);
WorldSOA PhysicsSystem::m_soa;

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

void SpatialGrid::rebuild(const std::vector<glm::vec2>& positions, const std::vector<entt::entity>& entities) {
    size_t numEntities = entities.size();

    cellIndices_.resize(numEntities);
    entitiesInCells.resize(numEntities);
    std::fill(counts_.begin(), counts_.end(), 0);

    for (size_t i = 0; i < numEntities; ++i) {
        int cellId = getCellId(positions[i]);
        cellIndices_[i] = cellId;
        counts_[cellId]++;
    }

    offsets[0] = 0;
    for (size_t i = 0; i < countCells; ++i) {
        offsets[i + 1] = offsets[i] + counts_[i];
    }

    std::fill(counts_.begin(), counts_.end(), 0);
    for (size_t i = 0; i < numEntities; ++i) {
        int cellId = cellIndices_[i];
        uint32_t destIdx = offsets[cellId] + counts_[cellId]++;
        entitiesInCells[destIdx] = entities[i];
    }
}

std::span<const entt::entity> SpatialGrid::entitiesInCell(int x, int y) const noexcept {
    int cellId = index(x, y, 0);
    size_t begin = offsets[cellId];
    size_t end = offsets[cellId + 1];
    return std::span<const entt::entity>(entitiesInCells.data() + begin, end - begin);
}

bool PhysicsSystem::tryPhagocyteConsumption(entt::registry& registry, entt::entity eater, entt::entity victim) {
    if (!registry.valid(eater) || !registry.valid(victim)) return false;
    if (eater == victim) return false;

    if (!registry.all_of<RenderData, Methabolism, Mass>(eater) || !registry.all_of<RenderData, Mass>(victim)) {
        return false;
    }

    auto* rdEater = registry.try_get<RenderData>(eater);
    auto* rdVictim = registry.try_get<RenderData>(victim);
    if (!rdEater || !rdVictim) return false;

    bool isPhagocyte = (rdEater->type == 0.0f);
    bool isDeadCell = (rdVictim->type == 99.0f);

    if (isPhagocyte && isDeadCell) {
        auto& metEater = registry.get<Methabolism>(eater);
        auto& massEater = registry.get<Mass>(eater);
        auto& massVictim = registry.get<Mass>(victim);
        
        massEater.value += 0.25f + massVictim.value * 1.0f;
        metEater.atf = std::min(metEater.maxAtf, metEater.atf + 50.0f);

        if (massEater.value > 15.0f) massEater.value = 15.0f;

        uint32_t idEater = static_cast<uint32_t>(entt::to_entity(eater));
        if (idEater < PhysicsSystem::m_soa.entityToIndex.size()) {
            size_t idxEater = PhysicsSystem::m_soa.entityToIndex[idEater];
            if (idxEater != SIZE_MAX) {
                PhysicsSystem::m_soa.mass[idxEater] = massEater.value;
            }
        }

        registry.destroy(victim);
        return true;
    }
    return false;
}

bool PhysicsSystem::tryDevourEnergy(entt::registry& registry, entt::entity eater, entt::entity victim, float dt) {
    if (!registry.valid(eater) || !registry.valid(victim)) return false;
    if (eater == victim) return false;

    if (!registry.all_of<Devorocite, Methabolism, Mass, RenderData>(eater) || 
        !registry.all_of<Methabolism, Mass, RenderData>(victim)) {
        return false;
    }

    auto* rdVictim = registry.try_get<RenderData>(victim);
    if (!rdVictim || rdVictim->type == 99.0f || rdVictim->type == 4.0f) return false;

    auto* devorocite = registry.try_get<Devorocite>(eater);
    auto* metEater = registry.try_get<Methabolism>(eater);
    auto* metVictim = registry.try_get<Methabolism>(victim);

    if (!devorocite || !metEater || !metVictim) return false;

    float desiredSteal = devorocite->stealRate * dt;
    float gainedAtf = 0.0f;
    float gainedMass = 0.0f;

    if (metVictim->atf > 0.0f) {
        gainedAtf = std::min(metVictim->atf, desiredSteal);
        metVictim->atf -= gainedAtf;
        desiredSteal -= gainedAtf;
    }

    if (!registry.valid(victim) || !registry.valid(eater) || 
        !registry.all_of<Mass>(victim) || !registry.all_of<Mass>(eater)) {
        return (gainedAtf > 0.0f);
    }

    if (desiredSteal > 0.0f) {
        auto& massVictim = registry.get<Mass>(victim);
        auto& massEater = registry.get<Mass>(eater);

        float massStealAmount = desiredSteal * 2.0f; 
        gainedMass = std::min(massVictim.value, massStealAmount);

        if (gainedMass > 0.0f) {
            massVictim.value -= gainedMass;
            massEater.value += gainedMass * devorocite->stealEfficiency;
        }
    }

    if (gainedAtf > 0.0f) {
        float effectiveAtfGain = gainedAtf * devorocite->stealEfficiency;
        
        metEater->atf += effectiveAtfGain;

        if (metEater->atf > metEater->maxAtf) {
            float excessAtf = metEater->atf - metEater->maxAtf;
            metEater->atf = metEater->maxAtf;

            if (registry.valid(eater) && registry.all_of<Mass>(eater)) {
                auto& massEater = registry.get<Mass>(eater);
                float efficiency = 0.95f;
                
                massEater.value += excessAtf * efficiency;
                if (massEater.value > 15.0f) massEater.value = 15.0f;

                uint32_t idEater = static_cast<uint32_t>(entt::to_entity(eater));
                if (idEater < PhysicsSystem::m_soa.entityToIndex.size()) {
                    size_t idxEater = PhysicsSystem::m_soa.entityToIndex[idEater];
                    if (idxEater != SIZE_MAX) {
                        PhysicsSystem::m_soa.mass[idxEater] = massEater.value;
                    }
                }
            }
        }
    }

    return (gainedAtf > 0.0f || gainedMass > 0.0f);
}

void PhysicsSystem::update(entt::registry& registry, const worldSettings& settings, float dt) {
    float subDt = dt / (float)subSteps;

    {
        ZONE_SCOPED("1.0. Pull from Registry to SOA");
        pullFromRegistry(registry);
    }

    {
        ZONE_SCOPED("1.2.1. Grid Rebuild");
        m_grid.rebuild(m_soa.pos, m_soa.entities);
    }

    for (int i = 0; i < subSteps; i++) {
        ZONE_SCOPED("1. PhysicsSystem Substep");

        {
            ZONE_SCOPED("1.1. applyForces (SOA)");
            applyForces(settings, subDt);
        }

        {
            ZONE_SCOPED("1.2. integratePosition (SOA)");
            integratePosition(settings, subDt);
        }

        {
            ZONE_SCOPED("1.3. resolveCollisions");
            resolveCollisions(registry, subDt);
        }
    }

    {
        ZONE_SCOPED("1.4. Push back to Registry");
        pushToRegistry(registry);
    }
}

void PhysicsSystem::pullFromRegistry(entt::registry& registry) {
    auto view = registry.view<Position, Velocity, Mass, Force, RenderData>();
    
    size_t count = registry.storage<Position>().size();

    m_soa.resize(count);

    size_t maxEntityId = 0;
    view.each([&](entt::entity entity, auto&, auto&, auto&, auto&, auto&) {
        maxEntityId = std::max(maxEntityId, static_cast<size_t>(entt::to_entity(entity)));
    });

    if (m_soa.entityToIndex.size() <= maxEntityId) {
        m_soa.entityToIndex.resize(maxEntityId + 1000, SIZE_MAX);
    }

    size_t idx = 0;
    view.each([&](entt::entity entity, auto& pos, auto& vel, auto& mass, auto& force, auto& rd) {
        m_soa.entities[idx] = entity;
        m_soa.pos[idx] = pos.value;
        m_soa.vel[idx] = vel.value;
        m_soa.force[idx] = force.value;
        m_soa.mass[idx] = mass.value;
        m_soa.radius[idx] = rd.radius;
        m_soa.renderType[idx] = rd.type;

        uint32_t id = static_cast<uint32_t>(entt::to_entity(entity));
        m_soa.entityToIndex[id] = idx;
        idx++;
    });
}

void PhysicsSystem::pushToRegistry(entt::registry& registry) {
    for (size_t i = 0; i < m_soa.entities.size(); ++i) {
        auto entity = m_soa.entities[i];
        if (!registry.valid(entity)) continue;

        if (registry.all_of<Position>(entity)) registry.get<Position>(entity).value = m_soa.pos[i];
        if (registry.all_of<Velocity>(entity)) registry.get<Velocity>(entity).value = m_soa.vel[i];
        if (registry.all_of<Force>(entity))    registry.get<Force>(entity).value = m_soa.force[i];
        if (registry.all_of<Mass>(entity))     registry.get<Mass>(entity).value = m_soa.mass[i];
    }
    
    for (size_t i = 0; i < m_soa.entities.size(); ++i) {
        uint32_t id = static_cast<uint32_t>(entt::to_entity(m_soa.entities[i]));
        if (id < m_soa.entityToIndex.size()) {
            m_soa.entityToIndex[id] = SIZE_MAX;
        }
    }
}

void PhysicsSystem::applyForces(const worldSettings& settings, float dt) {
    size_t count = m_soa.entities.size();
    for (size_t i = 0; i < count; ++i) {
        float m = m_soa.mass[i];
        glm::vec2& v = m_soa.vel[i];
        glm::vec2& f = m_soa.force[i];

        glm::vec2 acceleration = (f + glm::vec2(0.0f, settings.gravity * m) - (v * settings.viscosity)) / m;
        v += acceleration * dt;
        v *= glm::clamp(1.0f - settings.friction * dt, 0.0f, 1.0f);
        f = glm::vec2(0.0f);
    }
}

void PhysicsSystem::integratePosition(const worldSettings& settings, float dt) {
    size_t count = m_soa.entities.size();
    const float bounce = 0.5f;

    for (size_t i = 0; i < count; ++i) {
        glm::vec2& p = m_soa.pos[i];
        glm::vec2& v = m_soa.vel[i];

        p += v * dt;

        if (p.x < 0) { p.x = 0; v.x *= -bounce; }
        else if (p.x > settings.sizeX) { p.x = settings.sizeX; v.x *= -bounce; }
        
        if (p.y < 0) { p.y = 0; v.y *= -bounce; }
        else if (p.y > settings.sizeY) { p.y = settings.sizeY; v.y *= -bounce; }
    }
}

struct CollisionPair {
    size_t idxA;
    size_t idxB;
    entt::entity entityA;
    entt::entity entityB;
};

void PhysicsSystem::resolveCollisions(entt::registry& registry, float dt) {
    const int collisionIterations = 2;
    size_t count = m_soa.entities.size();

    thread_local std::vector<float> invMasses;
    invMasses.resize(count);
    for (size_t i = 0; i < count; ++i) {
        invMasses[i] = 1.0f / m_soa.mass[i];
    }

    thread_local std::vector<CollisionPair> pairs;
    {
        pairs.clear();
        pairs.reserve(count * 6);

        for (size_t i = 0; i < count; ++i) {
            entt::entity entityA = m_soa.entities[i];
            glm::vec2 posA = m_soa.pos[i];

            m_grid.forEachNeighborEntity(posA, [&](entt::entity entityB) {
                if (entityA >= entityB) return;

                uint32_t idB = static_cast<uint32_t>(entt::to_entity(entityB));
                if (idB >= m_soa.entityToIndex.size() || m_soa.entityToIndex[idB] == SIZE_MAX) return;
                size_t idxB = m_soa.entityToIndex[idB];

                float minDist = m_soa.radius[i] + m_soa.radius[idxB];
                float dx = posA.x - m_soa.pos[idxB].x;
                if (std::abs(dx) > minDist) return;
                float dy = posA.y - m_soa.pos[idxB].y;
                if (std::abs(dy) > minDist) return;

                float distSq = dx * dx + dy * dy;
                if (distSq < minDist * minDist && distSq > 0.000001f) {
                    pairs.push_back({i, idxB, entityA, entityB});
                }
            });
        }
    }

    for (int iter = 0; iter < collisionIterations; ++iter) {
        for (auto& pair : pairs) {
            size_t idxA = pair.idxA;
            size_t idxB = pair.idxB;

            if (iter == 0) {
                if (registry.valid(pair.entityA) && registry.valid(pair.entityB)) {
                    if (tryPhagocyteConsumption(registry, pair.entityA, pair.entityB)) continue;
                    if (tryPhagocyteConsumption(registry, pair.entityB, pair.entityA)) continue;

                    if (tryDevourEnergy(registry, pair.entityA, pair.entityB, dt)) continue;
                    if (tryDevourEnergy(registry, pair.entityB, pair.entityA, dt)) continue;
                } else {
                    continue;
                }
            }

            float minDist = m_soa.radius[idxA] + m_soa.radius[idxB];
            float dx = m_soa.pos[idxA].x - m_soa.pos[idxB].x;
            float dy = m_soa.pos[idxA].y - m_soa.pos[idxB].y;
            float distSq = dx * dx + dy * dy;

            if (distSq < minDist * minDist && distSq > 0.000001f) {
                float dist = std::sqrt(distSq);
                glm::vec2 normal = glm::vec2(dx, dy) / dist;
                float overlap = minDist - dist;

                float m1 = m_soa.mass[idxA];
                float m2 = m_soa.mass[idxB];
                float totalMass = m1 + m2;

                float m1Ratio = m2 / totalMass;
                float m2Ratio = m1 / totalMass;

                float pushFactor = 0.4f;
                m_soa.pos[idxA] += normal * (overlap * pushFactor) * m1Ratio;
                m_soa.pos[idxB] -= normal * (overlap * pushFactor) * m2Ratio;

                glm::vec2 relativeVel = m_soa.vel[idxA] - m_soa.vel[idxB];
                float velAlongNormal = glm::dot(relativeVel, normal);
                
                if (velAlongNormal < 0) {
                    float restitution = 0.2f;
                    float j = -(1.0f + restitution) * velAlongNormal;
                    j /= (invMasses[idxA] + invMasses[idxB]);
                    
                    glm::vec2 impulse = j * normal;
                    m_soa.vel[idxA] += invMasses[idxA] * impulse;
                    m_soa.vel[idxB] -= invMasses[idxB] * impulse;
                }
            }
        }
    }
}