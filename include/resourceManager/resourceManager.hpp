#pragma once

#include "resourceManager/cellConfigManager.hpp"
#include "resourceManager/genomeManager.hpp"
#include "resourceManager/worldManager.hpp"

class resourceManager {
private:
    WorldManager m_worldManager;
    cellcfgManager m_cellcfgManager;
    genomeManager m_genomeManager;

public:
    resourceManager() {
    }
    
    ~resourceManager() = default;

    WorldManager& getWorldManager() {
        return m_worldManager;
    }
    
    cellcfgManager& getCellsConfigManager() {
        return m_cellcfgManager;
    }
    
    genomeManager& getGenomeManager() {
        return m_genomeManager;
    }
};