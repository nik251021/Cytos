#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

struct NeuronChannelData {
    int inputNumber = 0;
    int outputNumber = 0;
    std::string formulaType = "multiply";
    float argument = 1.0f;
};

struct Module {
    std::string cell_type;
    float split_mass;
    std::map<std::string, int> childs;

    std::map<std::string, bool> flags;
    std::map<std::string, float> params;
    
    std::vector<NeuronChannelData> channels;

    float getParam(const std::string& key, float defaultVal) const {
        auto it = params.find(key);
        return (it != params.end()) ? it->second : defaultVal;
    }
};

struct Genome {
    std::string name;
    int startModule;
    std::vector<Module> modules;
};

class genomeManager {
private:
    std::unordered_map<std::string, Genome> m_registry;
    std::string m_genomesDir;

public:
    std::vector<std::string> getGenomeNames() const {
        std::vector<std::string> names;
        names.reserve(m_registry.size());
        for (const auto& [name, genome] : m_registry) {
            names.push_back(name);
        }
        return names;
    }

    const Genome& getGenome(const std::string& name) const {
        return m_registry.at(name);
    }
    
    explicit genomeManager(const std::string& dirPath = "data/gameData/genomes") 
        : m_genomesDir(dirPath) {
        loadAllFromDirectory(m_genomesDir);
    }

    void loadGenome(const std::string& name) {
        if (m_registry.find(name) != m_registry.end()) return;

        fs::path path = fs::path(m_genomesDir) / (name + ".json");
        std::ifstream file(path);
        
        if (!file.is_open()) {
            throw std::runtime_error("Could not open genome file: " + path.string());
        }

        nlohmann::json data;
        try {
            file >> data;
        } catch (const std::exception& e) {
            std::cerr << "JSON Parse error in " << path << ": " << e.what() << std::endl;
            throw;
        }

        Genome genome;
        genome.name = data.value("Name", "Unknown");
        genome.startModule = data.value("StartModule", 1);
        int totalModules = data.value("Modules_total", 0);
        
        for (int i = 1; i <= totalModules; ++i) {
            std::string modKey = "Module" + std::to_string(i);
            if (!data.contains(modKey)) continue;

            const auto& jMod = data[modKey];
            Module mod;

            try {
                mod.cell_type = jMod.value("cell_type", "default");
                mod.split_mass = jMod.value("split_mass", 1.0f);

                if (jMod.contains("channels") && jMod["channels"].is_array()) {
                    for (const auto& jChan : jMod["channels"]) {
                        NeuronChannelData cData;
                        cData.inputNumber = jChan.value("inputNumber", 0);
                        cData.outputNumber = jChan.value("outputNumber", 0);
                        cData.formulaType = jChan.value("formulaType", "multiply");
                        cData.argument = jChan.value("argument", 1.0f);
                        
                        mod.channels.push_back(cData);
                    }
                }

                if (jMod.contains("childs")) {
                    for (auto& [key, val] : jMod["childs"].items()) {
                        if (!val.is_number()) {
                            std::cerr << "CRITICAL: Key '" << key << "' in '" << path.string() 
                                      << "' is not a number! Value: " << val << std::endl;
                        }
                        mod.childs[key] = val.get<int>();
                    }
                }

                for (auto it = jMod.begin(); it != jMod.end(); ++it) {
                    std::string key = it.key();
                    if (key == "cell_type" || key == "split_mass" || key == "childs" || key == "channels") continue;
                    if (it->is_boolean()) mod.flags[key] = it.value();
                    else if (it->is_number()) mod.params[key] = it.value();
                }
            } catch (const std::exception& e) {
                std::cerr << "Error parsing module " << i << " in " << path.string() << ": " << e.what() << std::endl;
                throw;
            }
            genome.modules.push_back(mod);
        }

        m_registry[name] = std::move(genome);
        std::cout << "[GenomeManager] Loaded genome: " << name << std::endl;
    }

    void loadAllFromDirectory(const std::string& dirPath) {
        if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
            std::cerr << "[GenomeManager] Warning: Directory not found: " << dirPath << std::endl;
            return;
        }

        for (const auto& entry : fs::directory_iterator(dirPath)) {
            if (entry.is_regular_file()) {
                auto path = entry.path();
                if (path.extension() == ".json") {
                    std::string genomeName = path.stem().string();
                    try {
                        loadGenome(genomeName);
                    } catch (...) {
                    }
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