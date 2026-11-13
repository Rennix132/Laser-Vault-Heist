# Laser Vault Heist

Computer Graphics Fundamentals final project. C++17 and OpenGL 3.3 core.

Cross the laser corridor, collect the gem at the vault, then return to the green exit.

## Build on Windows

Use the MSYS2 UCRT64 terminal with GCC, CMake, Ninja and Git installed.
The first configure downloads GLFW 3.3.9 and GLM 1.0.1.

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/LaserVaultHeist.exe
```

## Controls

| Input | Action |
|---|---|
| W A S D | Walk |
| Mouse | Look |
| E | Collect the gem near the vault or escape near the green exit |
| F | Toggle the flashlight |
| R | Restart the run |
| Esc | Quit |

## Implemented features

- Indexed cube geometry used to build the corridor, pillars, laser emitters, vault and exit.
- Model, view and projection transforms with an animated vault door and a handle that follows its hinge.
- Per-fragment Blinn-Phong lighting with a fixed lamp, a moving light and a toggleable flashlight.
- Procedural floor tiles with derivative-based smoothing.
- Moving laser hazards, alarm and replay states, gem collection and extraction.
- Contextual prompts and FPS in the window title.

## Team

Add the names, groups and actual contributions before submission.

## AI Use

OpenAI Codex helped implement the vault scene, gameplay, lighting, procedural floor and documentation for the second milestone. Record any subsequent team edits in the final AI Use note.
