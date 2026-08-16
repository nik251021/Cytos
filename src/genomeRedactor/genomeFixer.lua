local config = require(src.genomeRedactor.config)

local genomeFixer = {}

function genomeFixer.fixNonExistChilds(genome)
    if genome.Modules then
        for id, module in pairs(genome.Modules) do
            if module.childs then
                local validChilds = {}
                for _, childId in ipairs(module.childs) do
                    if genome.Modules[tostring(childId)] then
                        table.insert(validChilds, childId)
                    else
                        print("Fixer: Deleted non extisted child " .. childId .. " from " .. id)
                        fixesApplied = fixesApplied + 1
                    end
                end
                module.childs = validChilds
            end
        end
    end
    return genome, fixesApplied
end

function genomeFixer.fixAll(genome)
    genomeFixer.fixNonExistChilds(genome)
end

function genomeFixer.fix(genome)
    if config.fixAll then
        genomeFixer.fixAll(genome)
        return
    end

    if config.fixNonExistChilds then
        genomeFixer.fixNonExistChilds(genome)
    end

end

return genomeFixer