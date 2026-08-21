#pragma once
#include <entt/entt.hpp>
#include <eventSystem/eventSystem.hpp>
#include <physics/components.hpp>
#include <algorithm>
#include <iostream>

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

        float frameBudget = devorocite->stealRate * dt;
        bool actionHappened = false;

        if (metVictim->atf > 0.0f) {
            float atfStealAmount = std::min(metVictim->atf, frameBudget);
            metVictim->atf -= atfStealAmount;
            
            float effectiveGain = atfStealAmount * devorocite->stealEfficiency;
            metEater->atf += effectiveGain;
            actionHappened = true;
        } 
        else if (registry.valid(victim) && registry.all_of<Mass>(victim)) {
            auto& massVictim = registry.get<Mass>(victim);
            auto& massEater = registry.get<Mass>(eater);

            float massStealAmount = std::min(massVictim.value, frameBudget * 0.2f);

            if (massStealAmount > 0.0f) {
                massVictim.value -= massStealAmount;
                massEater.value += massStealAmount * devorocite->stealEfficiency;
                if (massEater.value > 15.0f) massEater.value = 15.0f;
                actionHappened = true;
            }
        }

        if (metEater->atf > metEater->maxAtf) {
            float excessAtf = metEater->atf - metEater->maxAtf;
            metEater->atf = metEater->maxAtf;

            auto& massEater = registry.get<Mass>(eater);
            float atfToMassEfficiency = 0.025f;
            
            massEater.value += excessAtf * atfToMassEfficiency * dt;
            if (massEater.value > 15.0f) massEater.value = 15.0f;
        }

        return actionHappened;
    }
};