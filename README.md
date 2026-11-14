# Laser Vault Heist

Computer Graphics Fundamentals final project. C++17 and OpenGL 3.3 core.

Jump over the low laser, crouch beneath the high laser and climb the raised platform. Collect the gem at the vault, then cross the obstacles again to reach the green exit.

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
| Mouse | Look while captured |
| Tab | Release or capture the mouse |
| Left click in the game | Capture the mouse and resume play |
| Space | Jump |
| Ctrl (hold) | Crouch |
| E | Climb near the platform, collect the gem or escape |
| Enter or Continue | Close the level-two preview and return to the exit |
| Enter or Play Again | Restart from the result menu |
| F | Toggle the flashlight |
| R | Restart the run |
| Esc | Quit |

## Implemented features

- Indexed cube geometry used to build the corridor, pillars, laser emitters, vault and exit.
- Model, view and projection transforms with an animated vault door and a handle that follows its hinge.
- Per-fragment Blinn-Phong lighting from red laser beams, cyan neon strips and a toggleable flashlight.
- Procedural floor tiles with derivative-based smoothing.
- Fixed laser hazards, jumping, crouching, platform climbing, gem collection and extraction.
- In-game loss and victory menus with a Play Again button.
- Contextual prompts and FPS in the window title.

After the vault opens, the game pauses on a level-two preview. Choose Continue to resume the escape.
