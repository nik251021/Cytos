#pragma once
#include <entt/entt.hpp>
#include <eventSystem/eventSystem.hpp>
#include <physics/components.hpp>

class PhagocyteLogic {
private:
    static inline entt::registry* s_registry = nullptr;

public:
    static void init(entt::registry& registry) {
        s_registry = &registry;
        EventSystem::getDispatcher().sink<CollisionEvent>().connect<&PhagocyteLogic::onCollision>();
    }

    static void onCollision(const CollisionEvent& event) {
        if (!s_registry) return;
        
        tryConsume(*s_registry, event.entityA, event.entityB);
        tryConsume(*s_registry, event.entityB, event.entityA);
    }

private:
    static bool tryConsume(entt::registry& registry, entt::entity eater, entt::entity victim) {
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

            registry.destroy(victim);
            return true;
        }
        return false;
    }
};