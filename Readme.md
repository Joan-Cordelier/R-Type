# R-Type - Networked Game Engine

A multi-threaded networked implementation of the classic R-Type shoot'em'up game, built with a custom game engine architecture.

## Project Overview

This project implements a networked version of the classic R-Type game using advanced C++ techniques and software engineering practices. The game features:

- **Multi-threaded server** handling multiple concurrent players
- **Graphical client** with real-time networking
- **Custom game engine** with modular architecture
- **Cross-platform support** (Linux & Windows)

### Executables

- `r-type_server` - Game server handling multiplayer sessions
- `r-type_client` - Graphical game client

## Technical Stack

- **Language**: C++
- **Build System**: CMake
- **Package Manager**: vcpkg
- **Platforms**: Linux (required), Windows (recommended for cross-platform)

## Quick Start

### Prerequisites

- CMake 3.20+
- C++17 compatible compiler (GCC, Clang, MSVC)
- Git (for vcpkg)

### Building the Project

```bash
# Clone and setup vcpkg
git clone https://github.com/microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh  # On Linux/macOS
# OR
.\vcpkg\bootstrap-vcpkg.bat  # On Windows

# Configure CMake with vcpkg toolchain
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build build --config Release

# Run tests
cmake --build build --target test
```

### Running the Game

```bash
# Start server
./build/r-type_server

# Start client (in another terminal)
./build/r-type_client
```

## Project Structure

```
R-Type/
├── src/
│   ├── server/          # Server implementation
│   ├── client/          # Client implementation
│   ├── engine/          # Game engine core
│   ├── common/          # Shared code
│   └── ...
├── tests/               # Unit tests
├── docs/                # Documentation
├── .github/workflows/   # CI/CD pipelines
└── CMakeLists.txt       # Build configuration
```

---

## Git Flow Philosophy

Notre projet utilise une stratégie **Git Flow simplifié** avec deux branches principales :

- **`main`** : Branche de **production** contenant uniquement les releases stables et taggées
- **`dev`** : Branche **d'intégration** par défaut où tout le développement est mergé

```
┌─────────────────────────────────────────────────────────────┐
│                         PRODUCTION                          │
│  main ─●────────────●─────────────────●──────────>         │
│        │ v1.0.0     │ v1.1.0          │ v2.0.0              │
└─────────┼────────────┼─────────────────┼────────────────────┘
          │            │                 │
          │  Release   │    Release      │   Release
          │     PR     │       PR        │      PR
          │            │                 │
┌─────────┼────────────┼─────────────────┼────────────────────┐
│         │            │                 │   INTEGRATION      │
│  dev ───●────●───●───●─────●───●───●───●──────────>         │
│         │    │   │   │     │   │   │   │                    │
└─────────┼────┼───┼───┼─────┼───┼───┼───┼────────────────────┘
          │    │   │   └─────┘   │   │   └─ feature/mobile/notif
          │    │   └─────────────┘   └───── feature/front/calendar
          │    └─────────────────────────── bugfix/api/validation
          └──────────────────────────────── feature/back/auth

DÉVELOPPEMENT
feature/*, bugfix/*, hotfix/*, chore/*
```

### Pourquoi ce Workflow ?

 **Branche `main` propre** : Uniquement du code testé, validé et déployé en production
 **Intégration continue sur `dev`** : Détection rapide des conflits entre features
 **Releases contrôlées** : Chaque release est un point de contrôle avant production
 **Rollback facile** : Retour à une version stable via les tags Git
 **Historique clair** : `main` = historique des releases, `dev` = historique du développement

### Structure des Branches

```
main (production - releases only)
  ^
  │ PR de release (milestone atteint)
  │
dev (intégration - DEFAULT BRANCH)
  ^
  │ PRs quotidiennes
  │
feature/* / bugfix/* / hotfix/* (développement)
```

### Convention de Nommage des Branches

```bash
# Features - New game features or engine components
feature/<scope>/<description>
feature/engine/ecs-system
feature/network/udp-protocol
feature/client/rendering-pipeline
feature/server/room-manager
feature/gameplay/enemy-ai

# Bug Fixes
bugfix/<scope>/<description>
bugfix/network/packet-loss
bugfix/client/rendering-glitch

# Hotfixes (urgent, from main)
hotfix/<description>
hotfix/server-crash
hotfix/memory-leak

# Maintenance
chore/<description>
chore/update-dependencies
chore/ci-optimization
```

### Convention de Commit

We use **Conventional Commits** for clear project history:

```bash
<TYPE>(<scope>): <description courte>

Types:
ADD:      New feature or functionality
FIX:      Bug fix
REFACTOR: Code restructuring without behavior change
DOCS:     Documentation changes
TEST:     Adding or modifying tests
CHORE:    Maintenance tasks (dependencies, CI/CD)
STYLE:    Code formatting, whitespace
PERF:     Performance improvements
BUILD:    Build system or dependency changes

Scopes (examples):
server, client, engine, network, graphics, ecs, physics
```

**Examples:**
```bash
git commit -m "ADD(engine): implement ECS component system"
git commit -m "FIX(network): resolve packet deserialization bug"
git commit -m "REFACTOR(server): optimize game loop threading"
git commit -m "DOCS(api): add network protocol specification"
git commit -m "TEST(client): add rendering pipeline tests"
```

---

## Development Workflow

### 1. Create an Issue

Create an issue for each feature, bug, or task:
```
Issue #42: Implement ECS component system for entities
Issue #43: Fix network packet deserialization bug
```

### 2. Create Feature Branch from `dev`

```bash
git checkout dev
git pull origin dev
git checkout -b feature/engine/ecs-components
```

### 3. Develop with Regular Commits

```bash
git add .
git commit -m "ADD(engine): create base Component interface"
git commit -m "ADD(engine): implement Transform and Sprite components"
git commit -m "TEST(engine): add component registration tests"
```

### 4. Push and Create Pull Request to `dev`

```bash
git push origin feature/engine/ecs-components
```

**Pull Request Details:**
- **Title**: "ADD(engine): ECS component system"
- **Description**: 
  ```markdown
  Closes #42
  
  ## Changes
  - Implemented base Component interface
  - Added Transform, Sprite, and Physics components
  - Created ComponentManager for lifecycle management
  
  ## Testing
  - Unit tests for component registration
  - Integration tests with Entity system
  ```
- **Reviewers**: Assign team members based on scope
- **Labels**: enhancement, engine, tests

### 5. Code Review & CI/CD

- All automated checks must pass:
  - Coding style verification
  - CMake build (Linux)
  - Unit tests execution
- At least 1 approval required
- Address review comments
- Keep branch up-to-date with `dev`

### 6. Merge to `dev`

- Merge via GitHub after approval
- Issue closes automatically
- Feature branch deleted
- CI/CD validates integration

---

## Release Process

### When to Release

- **Part 1 Delivery** (Week 4): Core architecture + working prototype
- **Final Delivery** (Week 7): Advanced features
- Major milestone completions

### Release Steps

**1. Create Release Branch**
```bash
git checkout dev
git pull origin dev
git checkout -b release/v1.0.0
```

**2. Prepare Release**
```bash
# Update CMakeLists.txt: project(RType VERSION 1.0.0)
# Update CHANGELOG.md

git add .
git commit -m "RELEASE: prepare v1.0.0 - Part 1 Delivery"
git push origin release/v1.0.0
```

**3. Create PR: `release/v1.0.0` -> `main`**

```markdown
## Release v1.0.0 - Part 1 Delivery

### Features
- Multi-threaded server with room management
- UDP network protocol
- ECS-based game engine
- Client rendering with SFML
- Basic R-Type gameplay

### Architecture
- Component-based entity system
- Network serialization layer
- Game loop with fixed timestep

### Documentation
- Technical design document
- Network protocol specification
- Build guide
```

**4. After Merge: Tag Release**
```bash
git checkout main
git pull origin main
git tag -a v1.0.0 -m "Release v1.0.0 - Part 1: Core Architecture"
git push origin v1.0.0
```

**5. Merge Back: `main` -> `dev`**
```bash
git checkout dev
git merge main --no-ff -m "MERGE: sync main v1.0.0 into dev"
git push origin dev
```

**6. Automated Deployment**
- Builds Linux + Windows binaries
- Attaches to GitHub release
- Creates release notes

---

## Semantic Versioning

- **MAJOR** (v2.0.0): Breaking changes, incompatible API changes
- **MINOR** (v1.1.0): New features, backward compatible
- **PATCH** (v1.0.1): Bug fixes, backward compatible

**R-Type Project Timeline:**
```bash
v0.1.0  # Initial prototype - basic networking
v0.5.0  # Engine core - ECS implementation
v1.0.0  # Part 1 Delivery - Working multiplayer game
v1.1.0  # Advanced networking features
v1.2.0  # Enhanced gameplay mechanics
v2.0.0  # Final Delivery - Complete game with advanced features
```

---

## Branch Protection Rules

### Protection for `dev` Branch

The `dev` branch is protected with the following rules:

**Required Checks:**
- Coding style verification must pass
- CMake build must succeed
- All unit tests must pass
- At least 1 approval required
- Conversations must be resolved

**Restrictions:**
- No direct pushes (PRs only)
- No force pushes
- No branch deletion

### Protection for `main` Branch

The `main` branch has stricter protection:

**Required Checks:**
- All `dev` branch checks
- 2 approvals required (tech leads)
- Up-to-date with base branch
- Linear history enforced

**Restrictions:**
- No direct pushes (release PRs only)
- No force pushes
- No branch deletion
- Administrators cannot bypass

---

## Code Quality Standards

### Branch Naming Convention Enforcement

We automatically enforce branch naming conventions through GitHub Actions. The `.github/workflows/branch-naming.yml` workflow will block any PR from a branch that doesn't follow our conventions.

### Epitech Coding Style

We enforce Epitech coding style standards:
- Function length limits
- Code organization
- Naming conventions
- Documentation requirements

**Automated Checks:**
```yaml
# .github/workflows/ci.yml enforces:
- Branch naming convention validation
- Epitech coding style checker
- CMake compilation (Linux & Windows)
- Unit tests execution
```

### Code Review Checklist

Before approving a PR, reviewers should verify:

- [ ] Code follows Epitech coding style
- [ ] All new code has unit tests
- [ ] Documentation is updated
- [ ] No memory leaks (valgrind clean)
- [ ] Thread-safe implementation (for server code)
- [ ] Network protocol compatibility maintained
- [ ] Performance impact is acceptable
- [ ] Error handling is robust

---

## Dependencies & Build

### Package Manager Setup

The project uses **vcpkg** for dependency management:

```bash
# Clone vcpkg
git clone https://github.com/microsoft/vcpkg.git

# Bootstrap vcpkg
./vcpkg/bootstrap-vcpkg.sh  # Linux/macOS
.\vcpkg\bootstrap-vcpkg.bat  # Windows

# Dependencies are automatically installed during CMake configuration when using the vcpkg toolchain
```

### Required Dependencies

```json
# Example dependencies (to be configured in vcpkg.json)
{
  "dependencies": [
    "sfml",
    "asio",
    "catch2",
    "spdlog"
  ]
}
```

### Cross-Platform Build

**Linux:**
```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

**Windows (MSVC):**
```bash
cmake -B build -S . -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=.\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release
```

---

## Testing Strategy

### Unit Tests

```bash
# Run all tests
cmake --build build --target test

# Or use ctest
ctest --test-dir build --output-on-failure
```

### Test Coverage

- **Engine Components**: 80%+ coverage required
- **Network Protocol**: All packet types tested
- **Game Logic**: Critical paths covered
- **Server**: Concurrent client handling

### Integration Tests

- Client-server communication
- Multi-player scenarios
- Network packet serialization/deserialization
- Game state synchronization

---

## Documentation Requirements

### Required Documentation

1. **Technical Design Document**
   - System architecture overview
   - Network protocol specification
   - ECS architecture design
   - Threading model

2. **API Documentation**
   - Component interfaces
   - Network message formats
   - Public API reference

3. **Build & Deployment Guide**
   - Prerequisites
   - Build instructions (Linux/Windows)
   - Server deployment
   - Troubleshooting

4. **Developer Guide**
   - Project structure
   - Coding conventions
   - Adding new components
   - Debugging tips

---

## Deployment

### Server Deployment

```bash
# Build server
cmake --build build --target r-type_server

# Run server
./build/r-type_server --port 8080 --max-players 4
```

### Client Distribution

Release builds include:
- `r-type_server` (Linux/Windows)
- `r-type_client` (Linux/Windows)
- Required assets (textures, sounds)
- Configuration files
- README and documentation

---

## Team & Responsibilities

### Code Ownership

- **Engine Core** (ECS, Components): Assigned reviewers
- **Networking** (UDP, Protocol): Assigned reviewers
- **Client** (Graphics, Input): Assigned reviewers
- **Server** (Game Logic, Rooms): Assigned reviewers

### Review Assignments

| Component | Primary Reviewer | Secondary Reviewer |
|-----------|-----------------|-------------------|
| Engine/ECS | TBD | TBD |
| Networking | TBD | TBD |
| Client/Graphics | TBD | TBD |
| Server/Logic | TBD | TBD |

