#pragma once
#include <string>
#include <vector>
#include <map>
#include <functional>

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