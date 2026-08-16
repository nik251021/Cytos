# Cytos

**Cytos** - Simulator of multicellular organisms where you can make your own organisms like in cell lab

Project is written mostly in C++ with openGL, glfw, entt and nlohmanns json parser

In complect with this simulator you have a genome redactor written on lua in src/genomeRedactor

## Structure of project:
```text
cytos/
├── data/
│   └── gameData/
│       └── genomes/       # JSON-files with organsimes
├── src/
│   ├── genomeRedactor/    # Instruments for redactor written in lua
│   │   ├── main.lua       # main script
│   │   └── genomeValidator.lua # Validator of genomes
│   └── ...                # Main simulator
└── README.md              # You're here right now
```

## For start you need:
* Cmake
* C++ compiler
* Lua

## You can launch the project with commands:
```bash
./build/Cytos # For main simulation
lua src/genomeRedactor/main.lua # For redactor things
```

## Development and plans

* [x] Physical engine for cells
* [x] Validator of genomes
* [x] Auto fix of genome in redactor
* [x] Writing new shaders
* [x] Adding logic for phagocyte and adding dead cells (Cells which are not functional and can be eated with phagocyte)
* [] Adding devorocite (Cell which robs other cells and takes ATF), and ceratinocyte (Protector from devorocites which works like shield)