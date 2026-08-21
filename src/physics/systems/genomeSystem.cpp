#include <physics/systems/genomeSystem.hpp>
#include <physics/world.hpp>
#include <physics/components.hpp>
#include <cmath>

void GenomeSystem::update(world& w, entt::registry& registry, genomeManager& genomeManager, float dt) {
    auto cooldownView = registry.view<ReproductionCooldown>();
    for (auto entity : cooldownView) {
        auto& cd = cooldownView.get<ReproductionCooldown>(entity);
        if (cd.timer > 0.0f) {
            cd.timer -= dt;
        }
    }

    std::vector<entt::entity> toSplit;
    auto splitView = registry.view<Mass, SplitComponent, ReproductionCooldown>();
    
    splitView.each([&](auto entity, auto& mass, auto& split, auto& cd) {
        if (mass.value >= split.splitMass && cd.timer <= 0.0f) {
            toSplit.push_back(entity);
        }
    });

    for (auto e : toSplit) {
        if (registry.valid(e)) {
            forceSplit(w, registry, genomeManager, e);
        }
    }
}

void GenomeSystem::forceSplit(world& w, entt::registry& registry, genomeManager& genomeManager, entt::entity parent) {
    if (!registry.all_of<GenomeComponent, Position, Mass, Methabolism, SplitComponent>(parent)) {
        return;
    }

    auto& gen = registry.get<GenomeComponent>(parent);
    auto& pos = registry.get<Position>(parent);
    auto& mass = registry.get<Mass>(parent);
    auto& met  = registry.get<Methabolism>(parent);
    auto& split = registry.get<SplitComponent>(parent);

    const auto& mod = genomeManager.getModule(gen.genomeName, gen.currentModuleIndex);

    float splitRatio = mod.getParam("split_ratio", 0.5f);
    float restLength = mod.getParam("restLength", 7.5f);
    float maxLength  = mod.getParam("maxLength", 20.0f);
    float strength   = mod.getParam("strength", 250.0f);
    float splitAngle = mod.getParam("split_angle", 0.0f);

    float angleRad = glm::radians(splitAngle);
    glm::vec2 baseDir = glm::vec2(std::cos(angleRad), std::sin(angleRad));

    std::vector<entt::entity> children;

    for (auto const& [key, nextModuleIndex] : mod.childs) {
        float ratio = (key == "child1") ? splitRatio : (1.0f - splitRatio);
        float childMass = mass.value * ratio;
        float childAtf  = met.atf * ratio;

        glm::vec2 direction = (key == "child1") ? baseDir : -baseDir;
        glm::vec2 offset = direction * 10.f; 

        auto child = w.spawnCellFromModule(gen.genomeName, nextModuleIndex, pos.value + offset);
        
        if (child != entt::null) {
            registry.get<Mass>(child).value = childMass;
            
            auto* childMet = registry.try_get<Methabolism>(child);
            if (childMet) childMet->atf = childAtf;

            children.push_back(child);
        } else {
        }
    }

    if (!children.empty()) transferAdhesions(w, registry, parent, children);

    if (split.makeAdhesin && children.size() >= 2) {
        w.makeAdhesin(children[0], children[1], restLength, maxLength, strength);
    }

    registry.destroy(parent);
}

void GenomeSystem::transferAdhesions(world& w, entt::registry& registry, entt::entity oldParent, const std::vector<entt::entity>& children) {
    auto view = registry.view<Adhesion>();
    std::vector<Adhesion> newAdhesions;

    for (auto entity : view) {
        auto& adj = view.get<Adhesion>(entity);
        if (adj.cellA == oldParent || adj.cellB == oldParent) {
            entt::entity partner = (adj.cellA == oldParent) ? adj.cellB : adj.cellA;
            for (auto child : children) {
                newAdhesions.push_back({child, partner, adj.restLength, adj.maxLength, adj.strength});
            }
            registry.destroy(entity);
        }
    }
    for (auto& adj : newAdhesions) w.makeAdhesin(adj.cellA, adj.cellB, adj.restLength, adj.maxLength, adj.strength);
}