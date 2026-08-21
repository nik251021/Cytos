#pragma once
#include <entt/entt.hpp>

class BiologicalSystem {
public:
    static void update(entt::registry& registry, float dt, float lightAmount);

private:
    static void updateMetabolism(entt::registry& registry, float dt, float lightAmount);
    static void updateAdhesion(entt::registry& registry, float dt);
    static void processPendingDeaths(entt::registry& registry);
};