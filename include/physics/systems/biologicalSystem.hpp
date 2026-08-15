#pragma once
#include <entt/entt.hpp>

class BiologicalSystem {
public:
    static void update(entt::registry& registry, float dt);

private:
    static void updateMetabolism(entt::registry& registry, float dt);
    static void updateAdhesion(entt::registry& registry, float dt);
};