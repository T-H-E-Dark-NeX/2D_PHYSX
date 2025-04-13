# Infinite Maze Runner Game

A 2D maze game built using Raylib where players navigate through procedurally generated mazes while avoiding enemies and funky music.

# Project Overview

The game features:
- Procedurally generated mazes using SearchAndDestroy algorithm
- Enemy AI that chases the player
- Background music and sound effects
- Multiple difficulty levels
- Score tracking system

## Dependencies

External library used:
- Raylib (https://www.raylib.com/) - A simple lib.

## Development Environment Setup

# Windows
1. MSYS2 (Recommended):
   - Install MSYS2 from https://www.msys2.org/
   - Open MSYS2 terminal and run:
     ```bash
     pacman -S mingw-w64-x86_64-gcc
     pacman -S mingw-w64-x86_64-raylib
     ```
   - Add MinGW to your system PATH

2. Alternative: Use the provided raylib folder in the project

# Linux/Other OS
Replace the raylib folder with downloaded binaries for your OS from:
https://github.com/raysan5/raylib/releases/tag/5.5

## Compilation Instructions

### Windows
```terminal
gcc b24cm1070_b24me1067_b24ch1004_b24me1049_trial4.c b24cm1070_b24me1067_b24ch1004_b24me1049_music.c -o b24cm1070_b24me1067_b24ch1004_b24me1049_game.exe -Iraylib/include -Lraylib/lib -lraylib -lgdi32 -lwinmm -lm

```

### Linux
```bash
gcc b24cm1070_b24me1067_b24ch1004_b24me1049_trial4.c b24cm1070_b24me1067_b24ch1004_b24me1049_music.c -o b24cm1070_b24me1067_b24ch1004_b24me1049_game -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
```

 WebAssembly
1. Install Emscripten:
   ```bash
   git clone https://github.com/emscripten-core/emsdk.git
   cd emsdk
   ./emsdk install latest
   ./emsdk activate latest
   source ./emsdk_env.sh 
   ```

2. Compile for web:
   ```bash
   emcc b24cm1070_b24me1067_b24ch1004_b24me1049_trial4.c b24cm1070_b24me1067_b24ch1004_b24me1049_music.c -o web/game.html -I raylib/include -L raylib/lib -lraylib \
   -s USE_GLFW=3 -s WASM=1 -s ASYNCIFY -s TOTAL_MEMORY=67108864 \
   -s FORCE_FILESYSTEM=1 --preload-file assets --shell-file minimal.html
   ```


Note: WebAssembly build requires:
- Emscripten SDK
- Assets folder to be present in build directory
- Minimum 64MB memory allocation
- ASYNCIFY for audio support

## Project Structure
```
├── raylib/          # Raylib library files
├── assets/          # Game assets (images, sounds)
├── b24cm1070_b24me1067_b24ch1004_b24me1049_trial4.c         # Main game logic
├── b24cm1070_b24me1067_b24ch1004_b24me1049_music.c          # Music system implementation
└── README.md        # This file
```

## Controls
- Arrow keys: Move player
- ESC: Pause/Menu


## Contributors
- Tushar Verma: Maze generation algorithm, development environment setup, coordination
- Samyag Kothari: Music, maze generation integration
- Abhishek Sonparote: UI, player movement, enemy system
- Mrigank Sharma: Background image, game states, smooth gameplay




Licensed under HOT POTATO license. See license details.

