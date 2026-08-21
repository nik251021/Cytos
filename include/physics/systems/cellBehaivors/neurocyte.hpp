#pragma once
#include "physics/components.hpp"
#include <entt/entt.hpp>

class NeurocyteBehavior {
public:
    static void updateNeurocytes(entt::registry& registry, float dt) {
        auto view = registry.view<Signals, NeuronComponent>();

        view.each([&](entt::entity entity, auto& sigComp, auto& neuron) {
            for (const auto& channel : neuron.channells) {
                float inputVal = 0.0f;
                for (const auto& signal : sigComp.Signals) {
                    if (signal.number == channel.inputNumber) {
                        inputVal = signal.value;
                        break;
                    }
                }

                float processedVal = inputVal;
                if (channel.formule) {
                    processedVal = channel.formule(processedVal);
                }

                bool foundOutput = false;
                for (auto& targetSig : sigComp.Signals) {
                    if (targetSig.number == channel.outputNumber) {
                        targetSig.value = processedVal;
                        foundOutput = true;
                        break;
                    }
                }

                if (!foundOutput) {
                    sigComp.Signals.push_back({
                        .number = channel.outputNumber,
                        .value = processedVal
                    });
                }
            }
        });
    }
};