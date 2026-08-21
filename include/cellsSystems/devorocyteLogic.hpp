#pragma once
#include <entt/entt.hpp>
#include <eventSystem/eventSystem.hpp>
#include <physics/components.hpp>

class DevorocyteLogic {
private:
    static inline entt::registry* s_registry = nullptr;
    static inline float s_dt = 0.016f; 

public:
    static void init(entt::registry& registry) {
        s_registry = &registry;
        EventSystem::getDispatcher().sink<CollisionEvent>().template connect<&DevorocyteLogic::onCollision>();
    }
    
    static void setDeltaTime(float dt) { s_dt = dt; }

    static void onCollision(const CollisionEvent& event) {
        if (!s_registry) return;

        tryDevour(*s_registry, event.entityA, event.entityB, s_dt);
        tryDevour(*s_registry, event.entityB, event.entityA, s_dt);
    }

private:
    static bool tryDevour(entt::registry& registry, entt::entity eater, entt::entity victim, float dt) {
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
                }
            }
        }

        return (gainedAtf > 0.0f || gainedMass > 0.0f);
    }
};