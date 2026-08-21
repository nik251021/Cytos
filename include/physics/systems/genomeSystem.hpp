#pragma once
#include <entt/entt.hpp>
#include <resourceManager/resourceManager.hpp>

class world;

class GenomeSystem {
public:
    static void update(world& w, entt::registry& registry, genomeManager& genomeManager, float dt);
    static void forceSplit(world& w, entt::registry& registry, genomeManager& genomeManager, entt::entity parent);
private:
    static void transferAdhesions(world& w, entt::registry& registry, entt::entity oldParent, const std::vector<entt::entity>& children);
};