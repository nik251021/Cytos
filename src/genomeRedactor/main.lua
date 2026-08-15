--Currently indev
local genomeValidator = require("src.genomeRedactor.genomeValidator")

local genomeValidator = require("src.genomeRedactor.genomeValidator")

local errors, warnings, fileErr, genome = genomeValidator.validateFromFile("data/gameData/genomes/Genome1.json")

if fileErr then
    print("Error: " .. fileErr)
    return
end

print("--- results: " .. tostring(genome.Name) .. " ---")

print("Errors: " .. #errors)
for i, err in ipairs(errors) do
    print(" [X] " .. err)
end

print("warnings: " .. #warnings)
for i, warn in ipairs(warnings) do
    print(" [!] " .. warn)
end