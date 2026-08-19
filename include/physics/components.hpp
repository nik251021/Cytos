#pragma once
#include "glm/glm.hpp"
#include "physics/world.hpp"

struct Position {
    glm::vec2 value;
};
struct Rotation {
    float angle = 0.0f;
};
struct Velocity {
    glm::vec2 value;
};
struct Mass {
    float value;
};
struct Force {
    glm::vec2 value;
};
struct Methabolism {
    float atf;
    float maxAtf;
    float atfConsumptionRate;
    float massConsumptionRate;
    float efficiency = 1;
    float massEfficiency = 0.5;
    bool isActive = true;
};
struct SplitComponent {
    float splitMass;
    bool makeAdhesin;
};
struct GenomeComponent {
    std::string genomeName;
    int currentModuleIndex;
};
struct RenderData {
    glm::vec4 color;
    float radius;
    float type;
};

struct Adhesion {
    entt::entity cellA;
    entt::entity cellB;

    float restLength;
    float maxLength;
    float strength;
};

struct ReproductionCooldown {
    float timer = 0.0f;
    float duration = 1.0f;
};

struct Flagellum {
    float speed;
    float atfConsumption;
    bool enabled;
};

struct Devorocite {
    float stealRate = 50.0f;
    float stealEfficiency = 0.95f;
};

struct Keratinocite {};

//Signals
struct Signal {
    int number;
    float value;
};
struct Signals {
    float coefficient = 0.75f; // May be in settings of game in future
    std::vector<Signal> Signals;
};
//Neurocyte
struct NeuronChannel {
    int inputNumber;
    int outputNumber;

    std::function<float(float inputStrength)> formule;
};
struct NeuronComponent {
    std::vector<NeuronChannel> channells;
};