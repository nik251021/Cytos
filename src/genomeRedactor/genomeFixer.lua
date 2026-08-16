local config = require("src.genomeRedactor.config")

local genomeFixer = {}

function genomeFixer.fixNonExistChilds(genome)
    local fixesApplied = 0
    for key, module in pairs(genome) do
        if type(key) == "string" and key:match("^Module%d+$") then
            if module.childs and type(module.childs) == "table" then
                local validChilds = {}
                local childChanged = false
                
                for childName, targetId in pairs(module.childs) do
                    if genome["Module" .. targetId] then
                        validChilds[childName] = targetId
                    else
                        print(string.format("Fixer: Deleted non-existent child link '%s' -> %s from %s", tostring(childName), tostring(targetId), key))
                        fixesApplied = fixesApplied + 1
                        childChanged = true
                    end
                end
                
                if childChanged then
                    module.childs = validChilds
                end
            end
        end
    end
    return fixesApplied
end

function genomeFixer.fixMissingName(genome)
    if not genome.Name or type(genome.Name) ~= "string" then
        genome.Name = "Unnamed Organism"
        print("Fixer: Added default name 'Unnamed Organism'")
        return 1
    end
    return 0
end

function genomeFixer.fixModuleParameters(genome)
    local fixesApplied = 0

    for key, module in pairs(genome) do
        if type(key) == "string" and key:match("^Module%d+$") then
            if not module.cell_type or type(module.cell_type) ~= "string" then
                module.cell_type = "Photocyte"
                print("Fixer: Set default cell_type = 'Photocyte' for " .. key)
                fixesApplied = fixesApplied + 1
            end

            if not module.strength or module.strength <= 0 then
                module.strength = 1.0
                print("Fixer: Fixed invalid strength for " .. key)
                fixesApplied = fixesApplied + 1
            end

            if module.restLength and module.maxLength and module.restLength > module.maxLength then
                module.maxLength = module.restLength
                print("Fixer: Fixed restLength > maxLength for " .. key)
                fixesApplied = fixesApplied + 1
            end

            if not module.childs then
                module.childs = {}
                print("Fixer: Added missing childs table for " .. key)
                fixesApplied = fixesApplied + 1
            end
        end
    end

    return fixesApplied
end

function genomeFixer.fixStructureIntegrity(genome)
    local fixesApplied = 0
    local foundModules = 0
    local firstModuleKey = nil

    for key, _ in pairs(genome) do
        if type(key) == "string" and key:match("^Module%d+$") then
            foundModules = foundModules + 1
            if not firstModuleKey then
                firstModuleKey = tonumber(key:match("%d+"))
            end
        end
    end

    if genome.Modules_total ~= foundModules then
        local oldTotal = genome.Modules_total or "nil"
        genome.Modules_total = foundModules
        print(string.format("Fixer: Updated Modules_total from %s to %d", tostring(oldTotal), foundModules))
        fixesApplied = fixesApplied + 1
    end

    if not genome.StartModule or not genome["Module" .. genome.StartModule] then
        if firstModuleKey then
            genome.StartModule = firstModuleKey
            print("Fixer: Set StartModule to default existing " .. firstModuleKey)
            fixesApplied = fixesApplied + 1
        end
    end

    return fixesApplied
end

function genomeFixer.fixAll(genome)
    local totalFixes = 0
    totalFixes = totalFixes + genomeFixer.fixMissingName(genome)
    totalFixes = totalFixes + genomeFixer.fixStructureIntegrity(genome)
    totalFixes = totalFixes + genomeFixer.fixModuleParameters(genome)
    totalFixes = totalFixes + genomeFixer.fixNonExistChilds(genome)
    return totalFixes
end

function genomeFixer.fix(genome)
    print("Started fixing...")
    local totalFixes = 0

    if config.fixAll then
        totalFixes = genomeFixer.fixAll(genome)
    else
        if config.fixMissingName then
            totalFixes = totalFixes + genomeFixer.fixMissingName(genome)
        end
        if config.fixStructureIntegrity then
            totalFixes = totalFixes + genomeFixer.fixStructureIntegrity(genome)
        end
        if config.fixModuleParameters then
            totalFixes = totalFixes + genomeFixer.fixModuleParameters(genome)
        end
        if config.fixNonExistChilds then
            totalFixes = totalFixes + genomeFixer.fixNonExistChilds(genome)
        end
    end

    return genome, totalFixes
end

return genomeFixer