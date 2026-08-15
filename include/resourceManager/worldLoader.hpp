#pragma once

#include "external/nlohmann/json.hpp"

#include <fstream>
#include <filesystem>
#include <string>
#include <stdexcept>

namespace fs = std::filesystem;
const fs::path pathToWorlds("data/gameData/substrates");

struct worldSettings {
    std::string name;

    int max_cells;
    int max_food;
    float sizeX;
    float sizeY;
    float rad_lvl;
    float viscosity;
    float myo_disabled_friction;
    float myo_enabled_friction;
    float friction;
    float food_spawn_rate;
    float food_size;
    float salinity;
    float light_amount;
    float gravity;
};

inline worldSettings getWorldSettings(const std::string& fileName) {
    fs::path filePath = pathToWorlds / fs::path(fileName);
    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open world file: " + filePath.string());
    }

    nlohmann::json j;
    file >> j;

    worldSettings settings;
    
    settings.name = j.at("name").get<std::string>();
    settings.max_cells = j.at("max_cells").get<int>();
    settings.max_food = j.at("max_food").get<int>();
    
    settings.sizeX = j.at("size").at("size_x").get<float>();
    settings.sizeY = j.at("size").at("size_y").get<float>();
    
    settings.rad_lvl = j.at("rad_lvl").get<float>();
    settings.viscosity = j.at("viscosity").get<float>();
    settings.myo_disabled_friction = j.at("myo_disabled_friction").get<float>();
    settings.myo_enabled_friction = j.at("myo_enabled_friction").get<float>();
    settings.friction = j.at("friction").get<float>();
    settings.food_spawn_rate = j.at("food_spawn_rate").get<float>();
    settings.food_size = j.at("food_size").get<float>();
    settings.salinity = j.at("salinity").get<float>();
    settings.light_amount = j.at("light_amount").get<float>();
    settings.gravity = j.at("gravity").get<float>();

    return settings;
}