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
                    ++it;
                }
            }
        });

        auto adjView = registry.view<Adhesion>();
        adjView.each([&](entt::entity, auto& adj) {
            if (!registry.valid(adj.cellA) || !registry.valid(adj.cellB)) return;

            if (registry.all_of<Signals>(adj.cellA) && registry.all_of<Signals>(adj.cellB)) {
                auto& sigA = registry.get<Signals>(adj.cellA);
                auto& sigB = registry.get<Signals>(adj.cellB);

                for (const auto& signalA : sigA.Signals) {
                    if (signalA.value <= 0.05f) continue;

                    float transmittedValue = signalA.value * sigB.coefficient;

                    bool found = false;
                    for (auto& signalB : sigB.Signals) {
                        if (signalB.number == signalA.number) {
                            signalB.value = std::max(signalB.value, transmittedValue);
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        sigB.Signals.push_back({
                            .number = signalA.number,
                            .value = transmittedValue
                        });
                    }
                }

                for (const auto& signalB : sigB.Signals) {
                    if (signalB.value <= 0.05f) continue;

                    float transmittedValue = signalB.value * sigA.coefficient;

                    bool found = false;
                    for (auto& signalA : sigA.Signals) {
                        if (signalA.number == signalB.number) {
                            signalA.value = std::max(signalA.value, transmittedValue);
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
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