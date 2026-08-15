local dkjson = require("src.genomeRedactor.dkjson")

local GenomeValidator = {}

local function validateTable(genome)
    local errors = {}
    local warnings = {}
    
    if not genome.Name then
        table.insert(errors, "No genome name")
    end
    if not genome.StartModule or not genome["Module" .. genome.StartModule] then
        table.insert(errors, "Start module isnt valid")
    end
    if not genome.Modules_total then
        table.insert(errors, "No total count of modules")
    end
    
    local totalModules = genome.Modules_total or 0
    local foundModules = 0
    
    for key, module in pairs(genome) do
        if type(key) == "string" and key:match("^Module%d+$") then
            foundModules = foundModules + 1
            local modId = tonumber(key:match("%d+"))
            
            if not module.cell_type then
                table.insert(errors, string.format("module %d: No cell type", modId))
            end
            
            if module.strength and module.strength <= 0 then
                table.insert(errors, string.format("module %d: no strength", modId))
            end
            if module.restLength and module.maxLength and module.restLength > module.maxLength then
                table.insert(warnings, string.format("module %d: restLength is bigger then maxLength", modId))
            end
            
            if module.childs then
                for childName, targetId in pairs(module.childs) do
                    if not genome["Module" .. targetId] then
                        table.insert(errors, string.format("module %d (%s): no child module exist %d.", modId, childName, targetId))
                    end
                end
            else
                table.insert(warnings, string.format("Module %d: no childs", modId))
            end
        end
    end
    
    if foundModules ~= totalModules then
        table.insert(warnings, string.format("Total modules: %d, Found modules: %d.", totalModules, foundModules))
    end
    
    return errors, warnings
end

function GenomeValidator.validateFromFile(filepath)
    local file, openErr = io.open(filepath, "r")
    if not file then
        return nil, nil, "Failed to open file: " .. (openErr or "unknown error")
    end
    
    local content = file:read("*all")
    file:close()
    
    local genome, pos, err = dkjson.decode(content)
    if not genome then
        return nil, nil, "JSON syntax error at position " .. tostring(pos) .. ": " .. tostring(err)
    end
    
    local errors, warnings = validateTable(genome)
    
    return errors, warnings, nil, genome
end

return GenomeValidator