# Cyborg Collector 3D

A simple 3D game built using OpenGL (via the LearnOpenGL framework) in C++. In this game, you control a cyborg on a checkerboard playing field. The objective is to collect all the spinning planets while avoiding the scattered rocks. 

## Features

- **Player Character:** Control a 3D cyborg character.
- **Third-Person Camera:** Smooth camera that follows the player from behind and slightly above.
- **Collectibles:** Spinning and bobbing planets scattered across the map (collect them to increase your score!).
- **Obstacles:** Rocks of varying sizes that push the player back upon collision.
- **Lighting:** Phong lighting model (Ambient, Diffuse, Specular) with a directional light source.
- **Collision Detection:** Simple sphere-based collision detection for interacting with collectibles and obstacles.
- **Infinite Levels:** Collecting all planets advances you to the next level with a new fresh batch of collectibles.

## Tech Stack

- **C++17**
- **OpenGL 3.3 Core**
- **GLFW** for window creation and input handling
- **GLAD** for loading OpenGL function pointers
- **GLM** for mathematics and transformations
- **Assimp** for loading 3D models (`.obj`)
- **stb_image** for texture loading

## Build Instructions (Linux)

This relies on standard development libraries. You'll need CMake and a C++ compiler.

1. Install dependencies:
   ```bash
   sudo apt-get update
   sudo apt-get install cmake xorg-dev libgl1-mesa-dev pkg-config
   # Note: Assimp should preferably be available on your system, or CMake will try to find it.
   # sudo apt install libassimp-dev
   ```

2. Clone the repository and build:
   ```bash
   git clone git@github.com:frostnzx/Simple3Dgame.git
   cd Simple3Dgame

   # Create the build directory
   mkdir build && cd build
   
   # Configure the project
   cmake ..

   # Compile the project
   cmake --build . -j$(nproc)
   ```

3. Run the game:
   ```bash
   ./game
   ```

## Controls

- **W / S** — Move Forward / Backward
- **A / D** — Turn Left / Right
- **R** — Restart the current game (Resets score and level)
- **ESC** — Exit Game

## Project Structure

- `game.cpp`: Contains the main game loop, initialization, collision logic, input processing, and rendering.
- `game.vs` / `game.fs`: Vertex and Fragment shaders implementing the Phong lighting model and texturing.
- `CMakeLists.txt`: Build configuration linking against system libraries and LearnOpenGL utilities.
- Models and textures are loaded via Assimp from the included `resources` directory (e.g. cyborg, planets, rocks).

## Acknowledgments
Uses the framework and utility headers from [LearnOpenGL](https://learnopengl.com/). Models are sourced from the LearnOpenGL repository.
