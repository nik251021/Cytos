#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <fstream>

namespace fs = std::filesystem;

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

class cellcfgManager {
private:
    std::unordered_map<std::string, CellTemplate> m_cellTemplates;
    std::string m_configDir;

public:
    explicit cellcfgManager(const std::string& dirPath = "data/configs") 
        : m_configDir(dirPath) {
        loadAll(m_configDir);
    }

    CellTemplate loadCellTemplate(const std::string& path) {
        std::ifstream file(path);
        nlohmann::json j;
        
        if (!file.is_open()) {
            std::cerr << "[CellConfigManager] Can not open config file: " << path << std::endl;
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

    void loadAll(const std::string& dirPath) {
        if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
            std::cerr << "[CellConfigManager] Warning: Configs directory not found: " << dirPath << std::endl;
            return;
        }

        for (const auto& entry : fs::directory_iterator(dirPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                std::string pathStr = entry.path().string();
                CellTemplate t = loadCellTemplate(pathStr);
                m_cellTemplates[t.displayName] = t;
                std::cout << "[CellConfigManager] Auto-loaded cell config: " << t.displayName << std::endl;
            }
        }
    }

    const CellTemplate* getTemplate(const std::string& displayName) const {
        auto it = m_cellTemplates.find(displayName);
        if (it != m_cellTemplates.end()) {
            return &(it->second);
        }
        std::cerr << "[CellConfigManager] Error: Cell template not found: " << displayName << std::endl;
        return nullptr;
    }

    bool hasTemplate(const std::string& displayName) const {
        return m_cellTemplates.find(displayName) != m_cellTemplates.end();
    }

    const std::unordered_map<std::string, CellTemplate>& getAllTemplates() const {
        return m_cellTemplates;
    }
};