#pragma once
#include "physics/components.hpp"
#include <entt/entt.hpp>

class PhotocyteBehavior {
public:
    static void update(entt::registry& registry, entt::entity entity, Methabolism& met, Mass& mass, float lightAmount, float dt) {
        float energyProduction = lightAmount * 10.0f;
        
        float spaceForEnergy = met.maxAtf - met.atf;
        
        if (energyProduction * dt <= spaceForEnergy) {
            met.atf += energyProduction * dt;
        } else {
            met.atf = met.maxAtf;
            float excessEnergy = (energyProduction * dt) - spaceForEnergy;
            
            mass.value += excessEnergy * met.massEfficiency; 
        }
    }
};