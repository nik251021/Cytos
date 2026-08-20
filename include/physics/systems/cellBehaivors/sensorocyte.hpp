#pragma once
#include "physics/components.hpp"
#include <entt/entt.hpp>
#include <iostream>
#include <algorithm>

class SensorocyteSystem {
public:
    static void updateSensors(entt::registry& registry, float dt) {
        auto sensorView = registry.view<Position, Signals, SensorocyteComponent>();

        sensorView.each([&](entt::entity entity, auto& position, auto& sigComp, auto& sensor) {
            float sensedValue = 0.0f;

            switch (sensor.sensorType) {
                case 0: // Light
                    {
                        float normalizedY = (position.value.y + 400.0f) / 800.0f;
                        sensedValue = std::clamp(normalizedY * 10.0f * sensor.sensitivity, 0.0f, 10.0f);
                    }
                    break;

                case 1: // Velocity
                    if (registry.all_of<Velocity>(entity)) {
                        auto& vel = registry.get<Velocity>(entity);
                        sensedValue = glm::length(vel.value) * sensor.sensitivity;
                    }
                    break;

                case 2: // Mass
                    if (registry.all_of<Mass>(entity)) {
                        auto& mass = registry.get<Mass>(entity);
                        sensedValue = mass.value * sensor.sensitivity;
                    }
                    break;
                
                case 3: // ATF
                    if (registry.all_of<Methabolism>(entity)) {
                        auto& metab = registry.get<Methabolism>(entity);
                        sensedValue = metab.atf * sensor.sensitivity;
                    }
                    break;

                default:
                    sensedValue = 0.0f;
                    break;
            }

            if (sensedValue <= 0.01f) return;

            bool found = false;
            for (auto& signal : sigComp.Signals) {
                if (signal.number == sensor.outputNumber) {
                    signal.value = sensedValue;
                    found = true;
                    break;
                }
            }

            if (!found) {
                sigComp.Signals.push_back({
                    .number = sensor.outputNumber,
                    .value = sensedValue
                });
            }
        });
    }
};