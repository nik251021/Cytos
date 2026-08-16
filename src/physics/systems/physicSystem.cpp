#include <physics/systems/physicSystem.hpp>

SpatialGrid PhysicsSystem::m_grid;

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
        std::cout << ", Mass before: " << massEater.value << ", ATF: " << metEater.atf << "\n";
        massEater.value += 0.25f + massVictim.value * 1.0f;
        metEater.atf = std::min(metEater.maxAtf, metEater.atf + 50.0f);

        if (massEater.value > 15.0f) {
            massEater.value = 15.0f;
        }

        std::cout << ", Mass after: " << massEater.value << ", ATF: " << metEater.atf << "\n";

        registry.destroy(victim);
        return true;
    }

    return false;
}

void PhysicsSystem::update(entt::registry& registry, const worldSettings& settings, float dt) {
    applyForces(registry, settings, dt);
    integratePosition(registry, settings, dt);
    resolveCollisions(registry, dt);
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
    m_grid.clear();
    auto view = registry.view<Position, Velocity, Mass, RenderData>();
    
    view.each([&](auto entity, auto& pos, auto&, auto&, auto&) { 
        m_grid.add(entity, pos.value); 
    });

    view.each([&](auto entityA, auto& posA, auto& velA, auto& massA, auto& rdA) {
        int neighbors[9];
        m_grid.getNeighboringCells(posA.value, neighbors);
        
        for (int i = 0; i < 9; ++i) {
            auto it = m_grid.cells.find(neighbors[i]);
            if (it == m_grid.cells.end()) continue;

            for (auto entityB : it->second) {
                if (entityA >= entityB) continue;

                auto& posB = registry.get<Position>(entityB);
                auto& velB = registry.get<Velocity>(entityB);
                auto& massB = registry.get<Mass>(entityB);
                auto& rdB = registry.get<RenderData>(entityB);

                glm::vec2 delta = posA.value - posB.value;
                float distSq = glm::dot(delta, delta);
                float minDist = rdA.radius + rdB.radius;

                if (distSq < minDist * minDist && distSq > 0.000001f) {

                    if (tryPhagocyteConsumption(registry, entityA, entityB)) continue;
                    if (tryPhagocyteConsumption(registry, entityB, entityA)) continue;

                    float dist = std::sqrt(distSq);
                    glm::vec2 normal = delta / dist;
                    float overlap = minDist - dist;

                    float totalMass = massA.value + massB.value;
                    float m1Ratio = massB.value / totalMass;
                    float m2Ratio = massA.value / totalMass;

                    float pushFactor = 0.5f; 
                    velA.value += normal * (overlap * pushFactor) * m1Ratio;
                    velB.value -= normal * (overlap * pushFactor) * m2Ratio;

                    glm::vec2 relativeVel = velA.value - velB.value;
                    float velAlongNormal = glm::dot(relativeVel, normal);
                    
                    if (velAlongNormal < 0) {
                        float restitution = 0.3f;
                        float j = -(1.0f + restitution) * velAlongNormal;
                        j /= (1.0f / massA.value + 1.0f / massB.value);
                        
                        glm::vec2 impulse = j * normal;
                        velA.value += (1.0f / massA.value) * impulse;
                        velB.value -= (1.0f / massB.value) * impulse;
                    }
                }
            }
        }
    });
}