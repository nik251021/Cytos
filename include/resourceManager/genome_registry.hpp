#pragma once
#include <string>
#include <unordered_map>
#include "genome_types.hpp"
#include "genome_Loader.hpp"

class GenomeRegistry {
    std::unordered_map<std::string, Genome> m_registry;

public:
    void loadGenome(const std::string& name) {
        if (m_registry.find(name) == m_registry.end()) {
            m_registry[name] = GenomeLoader::load(name);
        }
    }

    const Module& getModule(const std::string& genomeName, int moduleIndex) const {
        return m_registry.at(genomeName).modules.at(moduleIndex - 1);
    }

    bool hasGenome(const std::string& name) const {
        return m_registry.find(name) != m_registry.end();
    }
};