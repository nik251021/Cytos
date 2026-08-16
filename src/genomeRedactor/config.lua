local config = {}

--Genome fixer
config.validateAndFix = true -- If enabled then genomeFixer.lua will be automatically enabled from genomeValidator.lua

config.fixAll = true --Apply all fixes in genomeFixer.lua
config.fixNonExistChilds = false --Fix childs which are not exist

return config