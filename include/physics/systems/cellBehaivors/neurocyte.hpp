#pragma once
#include "physics/components.hpp"
#include <entt/entt.hpp>

class NeurocyteBehavior {
public:
    static void updateNeurocytes(entt::registry& registry, float dt) {
        auto view = registry.view<Signals, NeuronComponent>();

        view.each([&](entt::entity entity, auto& sigComp, auto& neuron) {
            for (auto& signal : sigComp.Signals) {
                for (const auto& channel : neuron.channells) {
                    if (signal.number == channel.inputNumber) {
                        float oldVal = signal.value;
                        
                        if (channel.formule) {
                            signal.value = channel.formule(signal.value);
                        }
                        signal.number = channel.outputNumber;
                    }
                }
            }
        });
    }
};