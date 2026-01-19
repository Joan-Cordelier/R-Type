# R-Type - Multiplayer Shoot'em'up

A networked multiplayer implementation of the classic R-Type arcade game, featuring a custom ECS game engine and cross-platform support.


## Features

- **Multiplayer** : Up to 4 players in cooperative mode
- **4 Game Modes** : Classic, Boss Only, Endless, Speedy
- **4 Difficulty Levels** : Easy, Medium, Hard, Impossible
- **Account System** : Login, registration, and leaderboard
- **YAML Configuration** : Customize enemies, bosses, upgrades, and levels
- **Admin Console** : Server management with real-time stats

## Quick Start

```bash
# Clone the repository
git clone https://github.com/Joan-Cordelier/R-Type.git
cd R-Type

# Setup vcpkg
git clone https://github.com/microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh

# Build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build

# Run
./build/r-type_server        # Terminal 1
./build/r-type_client        # Terminal 2
```

## Documentation

**Full documentation available at: [https://joan-cordelier.github.io/R-Type/](https://joan-cordelier.github.io/R-Type/)**

| Section | Description |
|---------|-------------|
| [Installation](https://joan-cordelier.github.io/R-Type/user/installation.html) | Build prerequisites and compilation guide |
| [Usage](https://joan-cordelier.github.io/R-Type/user/usage.html) | How to run server and client |
| [Interface](https://joan-cordelier.github.io/R-Type/user/interface.html) | UI navigation and game modes |
| [Controls](https://joan-cordelier.github.io/R-Type/user/controls.html) | Keyboard controls |
| [YAML Config](https://joan-cordelier.github.io/R-Type/dev/yaml_config.html) | Configuration system for enemies, bosses, upgrades |
| [Network Protocol](https://joan-cordelier.github.io/R-Type/network/rfc.html) | RFC-style protocol specification |
| [ECS Architecture](https://joan-cordelier.github.io/R-Type/dev/ecs.html) | Entity Component System documentation |

## Project Structure

```
R-Type/
├── src/
│   ├── server/         # Game server (TCP/UDP)
│   ├── client/         # Graphical client (SDL2)
│   └── common/         # Shared code (ECS, Network, Config)
├── yaml/               # Game configuration files
├── textures/           # Sprites and assets
└── wiki/               # Documentation (mdbook)
```

## Tech Stack

- **C++17** with CMake and vcpkg
- **SDL2** (graphics, audio, input)
- **yaml-cpp** (configuration)
- **Custom ECS** engine
- **TCP/UDP** networking

## License

This project was developed as part of the Epitech curriculum.
