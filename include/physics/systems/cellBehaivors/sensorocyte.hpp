#pragma once
#include "physics/components.hpp"
#include <entt/entt.hpp>
#include <iostream>
#include <algorithm>
#include <glm/glm.hpp>

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

                case 5: // DistanceToColor
                    {
                        float minDist = sensor.maxRange;
                        bool targetFound = false;
                        auto targetView = registry.view<Position, RenderData>();
                        
                        targetView.each([&](entt::entity otherEntity, auto& otherPos, auto& otherRender) {
                            if (otherEntity == entity) return;

                            float colorDiff = glm::length(otherRender.color - sensor.targetColor);
                            if (colorDiff <= sensor.colorTolerance) {
                                float dist = glm::distance(position.value, otherPos.value);
                                if (dist < minDist) {
                                    minDist = dist;
                                    targetFound = true;
                                }
                            }
                        });

                        if (targetFound) {
                            float normalizedDist = 1.0f - (minDist / sensor.maxRange);
                            sensedValue = std::clamp(normalizedDist * 10.0f * sensor.sensitivity, 0.0f, 10.0f);
                        } else {
                            sensedValue = 0.0f;
                        }
                    }
                    break;

                case 6: // DistanceToType
                    {
                        float minDist = sensor.maxRange;
                        bool targetFound = false;
                        auto targetView = registry.view<Position, RenderData>();
                        
                        targetView.each([&](entt::entity otherEntity, auto& otherPos, auto& otherRender) {
                            if (otherEntity == entity) return;

                            if (otherRender.type == sensor.targetType) {
                                float dist = glm::distance(position.value, otherPos.value);
                                if (dist < minDist) {
                                    minDist = dist;
                                    targetFound = true;
                                }
                            }
                        });

                        if (targetFound) {
                            float normalizedDist = 1.0f - (minDist / sensor.maxRange);
                            sensedValue = std::clamp(normalizedDist * 10.0f * sensor.sensitivity, 0.0f, 10.0f);
                        } else {
                            sensedValue = 0.0f;
                        }
                    }
                    break;

                default:
                    sensedValue = 0.0f;
                    break;
            }

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