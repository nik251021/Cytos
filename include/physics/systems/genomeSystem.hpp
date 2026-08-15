#pragma once
#include <entt/entt.hpp>
#include <resourceManager/genome_registry.hpp>

class world;

class GenomeSystem {
public:
    static void update(world& w, entt::registry& registry, GenomeRegistry& genomeRegistry, float dt);
    static void forceSplit(world& w, entt::registry& registry, GenomeRegistry& genomeRegistry, entt::entity parent);
private:
    static void transferAdhesions(world& w, entt::registry& registry, entt::entity oldParent, const std::vector<entt::entity>& children);
};