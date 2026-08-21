#pragma once

#include "external/nlohmann/json.hpp"
#include <fstream>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <iostream>

struct WorldSettings {
    std::string name = "Default World";

    int max_cells = 1000;
    int max_food = 500;
    float sizeX = 1000.0f;
    float sizeY = 1000.0f;
    float rad_lvl = 0.0f;
    float viscosity = 0.3f;
    float myo_disabled_friction = 2.0f;
    float myo_enabled_friction = 1.0f;
    float friction = 3.5f;
    float food_spawn_rate = 0.5f;
    float food_size = 0.2f;
    float salinity = 0.8f;
    float light_amount = 1.0f;
    float gravity = 0.0f;
};

class WorldManager {
private:
    std::filesystem::path m_pathToWorlds;
    std::unordered_map<std::string, WorldSettings> m_loadedWorlds;

public:
    explicit WorldManager(std::string pathToWorldsDir = "data/gameData/substrates") 
        : m_pathToWorlds(pathToWorldsDir) {}

    const WorldSettings& getWorldSettings(const std::string& fileName) {
        auto it = m_loadedWorlds.find(fileName);
        if (it != m_loadedWorlds.end()) {
            return it->second;
        }

        std::filesystem::path filePath = m_pathToWorlds / fileName;
        std::ifstream file(filePath);
        
        WorldSettings settings;

        if (!file.is_open()) {
            std::cerr << "[WorldManager] Warning: Could not open world file: " << filePath 
                      << ". Using default settings.\n";
            return m_loadedWorlds[fileName] = settings;
        }

        nlohmann::json j;
        try {
            file >> j;
        } catch (const std::exception& e) {
            std::cerr << "[WorldManager] Error parsing JSON for world " << fileName 
                      << ": " << e.what() << ". Using defaults.\n";
            return m_loadedWorlds[fileName] = settings;
        }

        settings.name = j.value("name", "Unknown World");
        settings.max_cells = j.value("max_cells", 1000);
        settings.max_food = j.value("max_food", 500);
        
        if (j.contains("size") && j["size"].is_object()) {
            settings.sizeX = j["size"].value("size_x", 1000.0f);
            settings.sizeY = j["size"].value("size_y", 1000.0f);
        }
        
        settings.rad_lvl = j.value("rad_lvl", 0.0f);
        settings.viscosity = j.value("viscosity", 0.3f);
        settings.myo_disabled_friction = j.value("myo_disabled_friction", 2.0f);
        settings.myo_enabled_friction = j.value("myo_enabled_friction", 1.0f);
        settings.friction = j.value("friction", 3.5f);
        settings.food_spawn_rate = j.value("food_spawn_rate", 0.5f);
        settings.food_size = j.value("food_size", 0.2f);
        settings.salinity = j.value("salinity", 0.8f);
        settings.light_amount = j.value("light_amount", 1.0f);
        settings.gravity = j.value("gravity", 0.0f);

        std::cout << "[WorldManager] Successfully loaded world config: " << settings.name << std::endl;
        
        return m_loadedWorlds[fileName] = settings;
    }

    void clearCache() {
        m_loadedWorlds.clear();
    }
};