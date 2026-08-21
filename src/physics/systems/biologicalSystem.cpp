#include <physics/systems/biologicalSystem.hpp>
#include <physics/components.hpp>

#include <physics/systems/cellBehaivors/photocyte.hpp>
#include <physics/systems/cellBehaivors/flagellocyte.hpp>
#include <physics/systems/cellBehaivors/signals.hpp>
#include <physics/systems/cellBehaivors/neurocyte.hpp>
#include <physics/systems/cellBehaivors/sensorocyte.hpp>

#include <vector>

struct PendingDeath {
    entt::entity entity;
};

std::vector<PendingDeath> pendingDeaths;

void BiologicalSystem::update(entt::registry& registry, float dt, float lightAmount) {
    SignalsBehaivor::updateSignals(registry, dt);

    SensorocyteSystem::updateSensors(registry, dt);

    NeurocyteBehavior::updateNeurocytes(registry, dt);

    updateMetabolism(registry, dt, lightAmount);
    updateAdhesion(registry, dt);

    processPendingDeaths(registry);
}

void BiologicalSystem::processPendingDeaths(entt::registry& registry) {
    for (const auto& death : pendingDeaths) {
        if (!registry.valid(death.entity)) continue;
        
        auto* rd = registry.try_get<RenderData>(death.entity);
        if (rd) {
            rd->type = 99.0f;
            rd->color = glm::vec4(0.35f, 0.33f, 0.30f, 1.0f);
        }

        if (registry.all_of<Mass>(death.entity)) {
            auto& mass = registry.get<Mass>(death.entity);
            mass.value = 3.0f;
        }
        
        registry.remove<Methabolism>(death.entity);
        registry.remove<SplitComponent>(death.entity);
        
        if (registry.all_of<Flagellum>(death.entity)) registry.remove<Flagellum>(death.entity);
        if (registry.all_of<SensorocyteComponent>(death.entity)) registry.remove<SensorocyteComponent>(death.entity);
        if (registry.all_of<NeuronComponent>(death.entity)) registry.remove<NeuronComponent>(death.entity);
        if (registry.all_of<Signals>(death.entity)) registry.remove<Signals>(death.entity);
        if (registry.all_of<Devorocite>(death.entity)) registry.remove<Devorocite>(death.entity);
        if (registry.all_of<Keratinocite>(death.entity)) registry.remove<Keratinocite>(death.entity);
    }
    pendingDeaths.clear();
}

void BiologicalSystem::updateMetabolism(entt::registry& registry, float dt, float lightAmount) {
    thread_local std::vector<entt::entity> deadEntities;
    deadEntities.clear();

    auto view = registry.view<Methabolism, RenderData, Mass>();

    view.each([&](entt::entity entity, auto& met, auto& renderData, auto& mass) {
        if (renderData.type == 99.0f) return;

        if (mass.value > 15.0f) {
            mass.value = 15.0f;
        }

        if (renderData.type == 2.0f) {
            PhotocyteBehavior::update(registry, entity, met, mass, lightAmount, dt);
        }

        if (met.isActive) {
            if (met.atf > 0) {
                met.atf -= met.atfConsumptionRate * dt;
            } else {
                met.isActive = false;
            }
        } else {
            if (met.atf <= 0) {
                mass.value -= met.massConsumptionRate * dt;
                
                if (mass.value <= 0.5f) {
                    pendingDeaths.push_back({entity});
                    return;
                }
            } else {
                met.isActive = true;
            }
        }

        if (registry.all_of<Flagellum, Force, Rotation>(entity)) {
            auto& flag = registry.get<Flagellum>(entity);
            if (flag.enabled) {
                auto& force = registry.get<Force>(entity);
                auto rot = registry.get<Rotation>(entity);
                FlagellocyteBehavior::update(registry, entity, met, force, flag, rot, dt);
            }
        }
    });
}

void BiologicalSystem::updateAdhesion(entt::registry& registry, float dt) {
    auto view = registry.view<Adhesion>();
    thread_local std::vector<entt::entity> deadAdhesions;
    deadAdhesions.clear();

    view.each([&](entt::entity entity, auto& adj) {

        if (!registry.valid(adj.cellA) || !registry.valid(adj.cellB)) {
            deadAdhesions.push_back(entity);
            return;
        }

        if (!registry.all_of<Methabolism, RenderData>(adj.cellA) || 
            !registry.all_of<Methabolism, RenderData>(adj.cellB)) {
            deadAdhesions.push_back(entity);
            return;
        }

        auto* rdA = registry.try_get<RenderData>(adj.cellA);
        auto* rdB = registry.try_get<RenderData>(adj.cellB);
        if ((rdA && rdA->type == 99.0f) || (rdB && rdB->type == 99.0f)) {
            deadAdhesions.push_back(entity);
            return;
        }

        if (!registry.valid(adj.cellA) || !registry.valid(adj.cellB)) {
            deadAdhesions.push_back(entity);
            return;
        }

        auto& posA = registry.get<Position>(adj.cellA);
        auto& posB = registry.get<Position>(adj.cellB);
        auto& velA = registry.get<Velocity>(adj.cellA);
        auto& velB = registry.get<Velocity>(adj.cellB);

        glm::vec2 delta = posA.value - posB.value;
        float dist = glm::length(delta);

        if (dist > adj.maxLength) {
            deadAdhesions.push_back(entity);
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

    for (auto entity : deadAdhesions) {
        registry.destroy(entity);
    }
}