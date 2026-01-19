# Installation & Prérequis

Ce guide vous explique comment installer et compiler le jeu R-Type sur votre machine. Le projet utilise **CMake** et **vcpkg** pour garantir une compilation facile sur Linux.

---

## 1. Prérequis Système

Avant de commencer, assurez-vous d'avoir les outils suivants installés:

* **Git** : Pour cloner le dépôt.
* **CMake** (version 3.20 ou supérieure).
* **Ninja** (recommandé) ou Make.
* **Compilateur C++** compatible C++17 (GCC, Clang ou MSVC).
* **Dépendances graphiques** : OpenGL, X11/Wayland.

### Linux (Ubuntu/Debian/Fedora)

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install -y cmake build-essential ninja-build curl zip unzip tar \
    autoconf autoconf-archive automake libtool libltdl-dev \
    libgl1-mesa-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev \
    libxi-dev libxext-dev libwayland-dev libxkbcommon-dev

# Fedora
sudo dnf install cmake ninja-build gcc-c++ curl zip unzip tar \
    autoconf automake libtool mesa-libGL-devel libX11-devel libXrandr-devel \
    libXinerama-devel libXcursor-devel libXi-devel wayland-devel libxkbcommon-devel
```

---

## 2. Récupération du Projet

Clonez le dépôt officiel :

```bash
git clone https://github.com/EpitechPromo2026/B-CPP-500-LIL-5-1-rtype-nicolas.music.git
cd B-CPP-500-LIL-5-1-rtype-nicolas.music
```

---

## 3. Gestion des Dépendances (vcpkg)

Le projet utilise **vcpkg** pour gérer automatiquement les bibliothèques tierces (SDL2, yaml-cpp, etc.). Vous n'avez pas besoin de les installer manuellement.

Initialisez vcpkg :

```bash
# Sur Linux / macOS
git clone https://github.com/microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh
```

---

## 4. Compilation

Utilisez CMake pour configurer et compiler le projet.

### Linux / macOS

```bash
# Configuration (Release pour la performance)
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake

# Compilation
cmake --build build --config Release
```

---

## 5. Vérification

Une fois terminé, vérifiez que les exécutables sont présents :

```bash
# Linux
ls build/r-type_server build/r-type_client
```

---

## 6. Structure des Fichiers

Après compilation, votre dossier devrait contenir :

```
R-Type/
├── build/
│   ├── r-type_server      # Exécutable serveur
│   └── r-type_client      # Exécutable client
├── yaml/                  # Fichiers de configuration YAML
│   ├── main_loop.yaml     # Config principale du jeu
│   ├── upgrades.yaml      # Améliorations du joueur
│   └── enemies/           # Configuration des boss
├── textures/              # Assets graphiques
└── data/                  # Données utilisateurs (scores, etc.)
```

> **Important** : Le dossier `yaml/` doit être accessible depuis le répertoire d'exécution du serveur et du client.