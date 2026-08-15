#pragma once
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include "genome_types.hpp"

using json = nlohmann::json;

class GenomeLoader {
public:
    static Genome load(const std::string& genomeName) {
        std::string path = "data/gameData/genomes/" + genomeName + ".json";
        std::ifstream file(path);
        
        if (!file.is_open()) {
            throw std::runtime_error("Could not open genome file: " + path);
        }

        json data;
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

                if (jMod.contains("childs")) {
                    for (auto& [key, val] : jMod["childs"].items()) {
                        if (!val.is_number()) {
                            std::cerr << "CRITICAL: Key '" << key << "' in '" << path 
                                      << "' is not a number! Value: " << val << std::endl;
                        }
                        mod.childs[key] = val.get<int>();
                    }
                }

                for (auto it = jMod.begin(); it != jMod.end(); ++it) {
                    std::string key = it.key();
                    if (key == "cell_type" || key == "split_mass" || key == "childs") continue;
                    if (it->is_boolean()) mod.flags[key] = it.value();
                    else if (it->is_number()) mod.params[key] = it.value();
                }
            } catch (const std::exception& e) {
                std::cerr << "Error parsing module " << i << " in " << path << ": " << e.what() << std::endl;
                throw;
            }
            genome.modules.push_back(mod);
        }
        return genome;
    }
};