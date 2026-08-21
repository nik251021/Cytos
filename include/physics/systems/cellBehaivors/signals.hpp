#pragma once
#include "physics/components.hpp"
#include <entt/entt.hpp>
#include <iostream>
#include <algorithm>

class SignalsBehaivor {
public:
    static void updateSignals(entt::registry& registry, float dt) {
        auto signalsView = registry.view<Signals>();
        signalsView.each([&](entt::entity entity, auto& sigComp) {
            for (auto it = sigComp.Signals.begin(); it != sigComp.Signals.end(); ) {
                if (it->value <= 0.01f) {
                    it = sigComp.Signals.erase(it);
                } else {
                    it->value -= 0.02f * dt;
                    if (it->value <= 0.01f) {
                        it = sigComp.Signals.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
        });

        auto adjView = registry.view<Adhesion>();
        adjView.each([&](entt::entity, auto& adj) {
            if (!registry.valid(adj.cellA) || !registry.valid(adj.cellB)) return;

            if (registry.all_of<Signals>(adj.cellA) && registry.all_of<Signals>(adj.cellB)) {
                auto& sigA = registry.get<Signals>(adj.cellA);
                auto& sigB = registry.get<Signals>(adj.cellB);

                int sensorChannelA = -1;
                if (registry.all_of<SensorocyteComponent>(adj.cellA)) {
                    sensorChannelA = registry.get<SensorocyteComponent>(adj.cellA).outputNumber;
                }

                int sensorChannelB = -1;
                if (registry.all_of<SensorocyteComponent>(adj.cellB)) {
                    sensorChannelB = registry.get<SensorocyteComponent>(adj.cellB).outputNumber;
                }

                for (const auto& signalA : sigA.Signals) {
                    if (signalA.number == sensorChannelB) continue;

                    float transmittedValue = signalA.value * sigB.coefficient;

                    bool found = false;
                    for (auto& signalB : sigB.Signals) {
                        if (signalB.number == signalA.number) {
                            signalB.value = transmittedValue;
                            found = true;
                            break;
                        }
                    }
                    if (!found && transmittedValue > 0.01f) {
                        sigB.Signals.push_back({
                            .number = signalA.number,
                            .value = transmittedValue
                        });
                    }
                }

                for (const auto& signalB : sigB.Signals) {
                    if (signalB.number == sensorChannelA) continue;

                    float transmittedValue = signalB.value * sigA.coefficient;

                    bool found = false;
                    for (auto& signalA : sigA.Signals) {
                        if (signalA.number == signalB.number) {
                            signalA.value = transmittedValue;
                            found = true;
                            break;
                        }
                    }
                    if (!found && transmittedValue > 0.01f) {
                        sigA.Signals.push_back({
                            .number = signalB.number,
                            .value = transmittedValue
                        });
                    }
                }
            }
        });
    }
};