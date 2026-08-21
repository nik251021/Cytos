#pragma once
#include "physics/components.hpp"
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <cmath>
#include <algorithm>

class FlagellocyteBehavior {
public:
    static void update(entt::registry& registry, entt::entity entity, Methabolism& met, Force& force, Flagellum& flag, Rotation& rot, float dt) {
        if (!flag.enabled) return;

        float signalMultiplier = 1.0f;
        if (flag.inputNumber >= 0) {
            if (registry.all_of<Signals>(entity)) {
                auto& sigComp = registry.get<Signals>(entity);
                bool signalFound = false;
                
                for (const auto& signal : sigComp.Signals) {
                    if (signal.number == flag.inputNumber) {
                        signalMultiplier = std::clamp(signal.value, 0.0f, 1.0f);
                        signalFound = true;
                        break;
                    }
                }
                
                if (!signalFound) {
                    signalMultiplier = 0.0f;
                }
            } else {
                signalMultiplier = 0.0f;
            }
        }

        if (signalMultiplier <= 0.001f) return;

        float cost = flag.atfConsumption * signalMultiplier * dt;
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
            
            force.value += direction * flag.speed * signalMultiplier;
        }
    }
};