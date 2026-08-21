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
    int inputNumber = -1;
};

struct Devorocite {
    float stealRate = 100.0f;
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
//Sensors
enum class sensorType : int {
    Light = 0,
    Velocity = 1,
    Touch = 2,
    Energy = 3,
    Nutrient = 4,
    DistanceToColor = 5,
    DistanceToType = 6,
};

struct SensorocyteComponent {
    int sensorType = 0;
    int outputNumber = 0;
    float sensitivity = 1.0f;
    
    float maxRange = 200.0f;
    glm::vec4 targetColor{0.0f, 0.0f, 0.0f, 0.0f};
    float colorTolerance = 0.1f;
    float targetType = 0.0f;
};