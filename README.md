# Mini Racers 
A C++ top-down racing game inspired by Micro Machines, featuring two-player local multiplayer, checkpoint-based lap counting, and Box2D-powered physics with dynamic obstacles and boosters.

# Group

- Linh Bui
- Linh Vo
- Lam Ho
- Stanislaw Piskorski

# Repository organization
<details>
    <summary> Folder structure </summary>

```
./
├── assets/
│   ├── backgrounds/
│   ├── buttons/
│   ├── fonts/
│   ├── maps/
│   ├── obstacles/
│   ├── others/
│   ├── sounds/
│   │   ├── audio/
│   │   └── music/
│   └── vehicles/
├── doc/
├── libs/
├── plan/
├── src/
│   ├── Core/
│   │   ├── App.cpp
│   │   └── App.hpp
│   ├── Gameplay/
│   │   ├── Checkpoints/
│   │   │   ├── Checkpoint.cpp
│   │   │   ├── Checkpoint.hpp
│   │   │   ├── CheckpointManager.cpp
│   │   │   └── CheckpointManager.hpp
│   │   ├── Obstacles/
│   │   │   ├── HardObstacle.hpp
│   │   │   ├── SoftObstacle.hpp
│   │   │   ├── SoftObstacleInteraction.cpp
│   │   │   ├── SoftObstacleInteraction.hpp
│   │   │   ├── SoftObstacleManager.cpp
│   │   │   └── SoftObstacleManager.hpp
│   │   ├── GameWorld.hpp
│   │   ├── Vehicle.cpp
│   │   ├── Vehicle.hpp
│   │   ├── VehicleSpec.cpp
│   │   └── VehicleSpec.hpp
│   ├── Race/
│   │   ├── Camera.cpp
│   │   ├── Camera.hpp
│   │   ├── Countdown.cpp
│   │   ├── Countdown.hpp
│   │   ├── HUD.cpp
│   │   ├── HUD.hpp
│   │   ├── MapObjectParser.cpp
│   │   ├── MapObjectParser.hpp
│   │   ├── Timer.hpp
│   │   ├── Track.cpp
│   │   └── Track.hpp
│   ├── Scenes/
│   │   ├── Components/
│   │   │   └── Button/
│   │   │       ├── BtnContainer.cpp
│   │   │       ├── BtnContainer.hpp
│   │   │       ├── Button.cpp
│   │   │       └── Button.hpp
│   │   ├── MenuScene.cpp
│   │   ├── MenuScene.hpp
│   │   ├── PauseScene.cpp
│   │   ├── PauseScene.hpp
│   │   ├── RaceScene.cpp
│   │   ├── RaceScene.hpp
│   │   ├── Scene.hpp
│   │   ├── SelectMapScene.cpp
│   │   ├── SelectMapScene.hpp
│   │   ├── VictoryScene.cpp
│   │   └── VictoryScene.hpp
│   ├── System/
│   │   ├── AudioManager.cpp
│   │   ├── AudioManager.hpp
│   │   ├── Config.hpp
│   │   ├── GameContext.hpp
│   │   ├── InputManager.hpp
│   │   ├── SceneManager.cpp
│   │   └── SceneManager.hpp
│   ├── main.cpp
├── tests/
├── CMakeLists.txt
└── README.md
```

</details>

# Features
## Core Gameplay Features
- Local two-player split-screen multiplayer
- Checkpoint-based lap counting system
- Boost pads that increase speed
- Oil and off-track surfaces that reduce traction and speed
- Hard and soft obstacles affecting vehicle movement

## Vehicle & Physics Features
- Configurable vehicle tuning (speed, acceleration)
- Collision handling with walls, obstacles, and other vehicles

## Track & Environment Features
- TMX-based track loading using Tiled maps
- Dynamic obstacle parsing from TMX object layers
- Multiple selectable race tracks
- Off-track detection with speed penalty

## User Interface Features
- Main menu with navigation and selection
- In-race HUD showing laps progress
- Victory screen displaying race results
- Pause menu during races
- Map selection menu

## Audio Features
- Background music during gameplay
- Sound effects for driving and game events

# Installation

1. Install dependencies

```bash
sudo apt update
sudo apt install build-essential cmake libsfml-dev
```

2. Clone repo

```bash
git clone https://version.aalto.fi/gitlab/cpp-2025/micro-machines-1.git
cd micro-machines-1
```

3. Build and run the game

```bash
mkdir build && cd build
cmake .. && make && ./MicroMachines
```

# How to play
### Menu:
- Arrow keys to navigate
- Enter to select
- ESC to go back

### Player Controls:
- Player 1 (left view): WASD
- Player 2 (right view): Arrow keys
- ESC to pause 
