#include <physics/systems/biologicalSystem.hpp>
#include <physics/components.hpp>
#include <physics/systems/cellBehaivors/photocyte.hpp>
#include <physics/systems/cellBehaivors/flagellocyte.hpp>

void BiologicalSystem::update(entt::registry& registry, float dt) {
    updateMetabolism(registry, dt);
    updateAdhesion(registry, dt);
}

void BiologicalSystem::updateMetabolism(entt::registry& registry, float dt) {
    registry.view<Methabolism, RenderData, Mass>().each([&](auto entity, auto& met, auto &renderData, auto & mass) {
        if (renderData.type == 2.0f) {
            PhotocyteBehavior::update(registry, entity, met, mass, 1.0f, dt);
        }
        if (met.isActive) {
            if (met.atf > 0) {
                met.atf -= met.atfConsumptionRate * dt;
            } else {
                met.isActive = false;
            }
        } else {
            if (met.atf <= 0) {
                auto& mass = registry.get<Mass>(entity);
                mass.value -= met.massConsumptionRate * dt;
                if (mass.value <= 0) {
                    registry.destroy(entity);
                }
            } else {
                met.isActive = true;
            }
        }
    });
    registry.view<Methabolism, Force, Flagellum, Rotation>().each([&](auto entity, auto& met, auto& force, auto& flag, auto rot) {
        if (flag.enabled) {
            FlagellocyteBehavior::update(registry, entity, met, force, flag, rot, dt);
        }
    });
}

void BiologicalSystem::updateAdhesion(entt::registry& registry, float dt) {
    auto view = registry.view<Adhesion>();

    view.each([&](auto entity, auto& adj) {
        if (!registry.valid(adj.cellA) || !registry.valid(adj.cellB)) {
            registry.destroy(entity);
            return;
        }

        auto& posA = registry.get<Position>(adj.cellA);
        auto& posB = registry.get<Position>(adj.cellB);
        auto& velA = registry.get<Velocity>(adj.cellA);
        auto& velB = registry.get<Velocity>(adj.cellB);

        glm::vec2 delta = posA.value - posB.value;
        float dist = glm::length(delta);

        if (dist > adj.maxLength) {
            registry.destroy(entity);
            return;
        }

        glm::vec2 direction = (dist > 0.0001f) ? (delta / dist) : glm::vec2(0.0f);
        float springForce = adj.strength * (dist - adj.restLength);

        glm::vec2 relativeVel = velA.value - velB.value;
        float dampingFactor = 10.0f;
        float dampingForce = glm::dot(relativeVel, direction) * dampingFactor;

        float totalForceMag = springForce + dampingForce;
        glm::vec2 force = -totalForceMag * direction;

        registry.get<Force>(adj.cellA).value += force;
        registry.get<Force>(adj.cellB).value -= force;

        auto& metA = registry.get<Methabolism>(adj.cellA);
        auto& metB = registry.get<Methabolism>(adj.cellB);
        
        float transferRate = 5.0f;
        float diff = (metA.atf - metB.atf) * transferRate * dt;
        
        metA.atf -= diff;
        metB.atf += diff;
    });
}