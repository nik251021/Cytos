local dkjson = require("src.genomeRedactor.dkjson")
local fixer = require("src.genomeRedactor.genomeFixer")

local GenomeValidator = {}

local function validateTable(genome)
    local errors = {}
    local warnings = {}
    
    if not genome.Name or type(genome.Name) ~= "string" then
        table.insert(errors, "No genome name or invalid type")
    end
    
    if not genome.StartModule then
        table.insert(errors, "No StartModule defined")
    elseif not genome["Module" .. genome.StartModule] then
        table.insert(errors, string.format("StartModule points to non-existent Module%d", genome.StartModule))
    end
    
    if not genome.Modules_total then
        table.insert(errors, "No total count of modules (Modules_total)")
    end
    
    local totalModules = genome.Modules_total or 0
    local foundModules = 0
    
    for key, module in pairs(genome) do
        if type(key) == "string" and key:match("^Module%d+$") then
            foundModules = foundModules + 1
            local modId = tonumber(key:match("%d+"))
            
            if not module.cell_type or type(module.cell_type) ~= "string" then
                table.insert(errors, string.format("module %d: Missing or invalid cell_type (must be a string)", modId))
            end
            
            if module.strength and module.strength <= 0 then
                table.insert(errors, string.format("module %d: strength must be > 0", modId))
            end
            
            if module.restLength and module.maxLength and module.restLength > module.maxLength then
                table.insert(warnings, string.format("module %d: restLength is greater than maxLength", modId))
            end
            
            if module.childs and type(module.childs) == "table" then
                for childName, targetId in pairs(module.childs) do
                    if not genome["Module" .. targetId] then
                        table.insert(errors, string.format("module %d (%s): non-existent child module %d", modId, childName, targetId))
                    end
                end
            else
                table.insert(warnings, string.format("Module %d: missing or empty childs table", modId))
            end
        end
    end
    
    if foundModules ~= totalModules then
        table.insert(warnings, string.format("Modules_total mismatch: declared %d, found %d", totalModules, foundModules))
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
        return nil, nil, "JSON syntax error at position " .. tostring(pos) .. ":" .. tostring(err)
    end
    
    local fixedGenome, totalFixes = fixer.fix(genome)
    
    if totalFixes > 0 then
        print(string.format("Found and fixed %d issues. Saving changes...", totalFixes))
        
        local backupPath = filepath .. ".bak"
        local backupFile = io.open(backupPath, "w")
        if backupFile then
            backupFile:write(content)
            backupFile:close()
        end
        
        local newContent = dkjson.encode(fixedGenome, { indent = true })
        local newFile = io.open(filepath, "w")
        if newFile then
            newFile:write(newContent)
            newFile:close()
        end
    end
    
    local errors, warnings = validateTable(fixedGenome)
    return errors, warnings, nil, fixedGenome
end

return GenomeValidator