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

* [x] Validator of genomes
* [x] Auto fix of genome in redactor
* [x] Finish neurocytes, add axonocytes
* [x] Add sensors -- In progress
* [x] Adding inspect on mousebutton2
* [x] Rewrite of resourceManager
* [x] Event system
* [x] Genome spawn button
* [x] Add input blocks and modificate current (Flagellocytes, myocytes and etc.) -- In progress, Flagellocytes are done
* [x] Add eye in the game and new sensor type for cells -- In progress
* [] Random genome generator in redactor
* [] Redactor GUI and pre-simulation view
* [] Collision fix