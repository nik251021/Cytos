#pragma once
#include "physics/components.hpp"
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <cmath>

class FlagellocyteBehavior {
public:
    static void update(entt::registry& registry, entt::entity entity, Methabolism& met, Force& force, Flagellum& flag, Rotation& rot, float dt) {
        float cost = flag.atfConsumption * dt;
        if (met.atf >= cost) {
            met.atf -= cost;
            
            float finalAngle = rot.angle;

            auto view = registry.view<Adhesion>();
            view.each([&](auto, auto& adj) {
                entt::entity other = entt::null;
                if (adj.cellA == entity) other = adj.cellB;
                else if (adj.cellB == entity) other = adj.cellA;

                if (other != entt::null && registry.valid(other) && registry.all_of<Position>(other)) {
                    glm::vec2 myPos = registry.get<Position>(entity).value;
                    glm::vec2 otherPos = registry.get<Position>(other).value;

                    glm::vec2 r = myPos - otherPos;
                    if (glm::length(r) > 0.001f) {
                        float colonyOrientation = std::atan2(r.y, r.x);
                        finalAngle += colonyOrientation;
                    }
                }
            });

            glm::vec2 direction(std::cos(finalAngle), std::sin(finalAngle));
            
            force.value += direction * flag.speed;
        }
    }
};