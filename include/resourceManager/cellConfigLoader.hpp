#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <fstream>

struct CellTemplate {
    std::string displayName;
    float maxRadius;
    float Default_mass;
    float typeId;
    float maxCollisionStrength;
    
    float maxAtf;
    float atfConsumptionRate;
    float massConsumptionRate;
};

inline CellTemplate loadCellTemplate(const std::string& path) {
    std::ifstream file(path);
    nlohmann::json j;
    
    if (!file.is_open()) {
        std::cerr << "Can not open config file: " << path << std::endl;
        return {}; 
    }
    file >> j;

    CellTemplate t;
    t.displayName = j.value("Display_name", "Unknown");
    t.maxRadius = j.value("Max_radius", 5.0f);
    t.Default_mass = j.value("Default_mass", 1.0f);
    t.typeId = j.value("typeid", 0.0f);
    t.maxCollisionStrength = j.value("Max_collision strength", 1000.0f);
    
    auto metab = j.value("Metabolism", nlohmann::json::object());
    t.maxAtf = metab.value("maxAtf", 100.0f);
    t.atfConsumptionRate = metab.value("atfConsumptionRate", 0.1f);
    t.massConsumptionRate = metab.value("massConsumptionRate", 0.1f);
    
    return t;
}