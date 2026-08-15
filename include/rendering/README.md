# Hello
## If you are reading this, you are maybe a creator of mods or a contributor.

### IMPORTANT: Architectural Integrity
PLEASE, for your own sanity and the project's health: **DO NOT WRITE LOGIC OR PHYSICS HERE.**
* If it’s physics/movement, put it in `src/physics/` or `include/physics/`.
* If it’s cell logic like neurocytes, put it in `src/logic/` or `include/logic/`.
* If it’s rendering/graphics, put it in `src/rendering/` or `include/rendering/`. (Here)

We follow a strict "Bridge" pattern:
1. Physics/Logic engines calculate the state.
2. The Bridge/Systems translate that state into `RenderData`.
3. The Renderer blindly draws what it's told.

### Project Structure Guide
* `src/` — Main implementation (C++ source files).
* `include/` — Public headers and library definitions.
* `data/` — Non-compiled assets (JSON configs, GLSL shaders).
* `include/external/` — Third-party libraries (GLM, EnTT, GLFW, etc.).

### Current Status
* [ ] Started rewriting of the rendering