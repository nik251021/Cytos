#pragma once
#include <string>
#include <unordered_map>
#include <filesystem>
#include <iostream>
#include "genome_types.hpp"
#include "genome_Loader.hpp"

namespace fs = std::filesystem;

class GenomeRegistry {
    std::unordered_map<std::string, Genome> m_registry;

public:
    GenomeRegistry() {
        loadAllFromDirectory("data/gameData/genomes");
    }

    void loadGenome(const std::string& name) {
        if (m_registry.find(name) == m_registry.end()) {
            try {
                m_registry[name] = GenomeLoader::load(name);
                std::cout << "[GenomeRegistry] Loaded genome: " << name << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[GenomeRegistry] Error loading genome '" << name << "': " << e.what() << std::endl;
            }
        }
    }

    void loadAllFromDirectory(const std::string& dirPath) {
        if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
            std::cerr << "[GenomeRegistry] Warning: Directory not found: " << dirPath << std::endl;
            return;
        }

        for (const auto& entry : fs::directory_iterator(dirPath)) {
            if (entry.is_regular_file()) {
                auto path = entry.path();
                if (path.extension() == ".json") {
                    std::string genomeName = path.stem().string();
                    loadGenome(genomeName);
                    std::cout << "[GenomeRegistry] Auto-loaded: " << genomeName << std::endl;
                }
            }
        }
    }

    const Module& getModule(const std::string& genomeName, int moduleIndex) const {
        return m_registry.at(genomeName).modules.at(moduleIndex - 1);
    }

    bool hasGenome(const std::string& name) const {
        return m_registry.find(name) != m_registry.end();
    }
};