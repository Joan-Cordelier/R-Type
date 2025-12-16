# Installation & Prérequis

Ce guide vous explique comment installer et compiler le jeu R-Type sur votre machine. Le projet utilise **CMake** et **vcpkg** pour garantir une compilation facile sur Linux et Windows.

## 1. Prérequis Système

Avant de commencer, assurez-vous d'avoir les outils suivants installés:

* **Git** : Pour cloner le dépôt.
* **CMake** (version 3.20 ou supérieure).
* **Compilateur C++** compatible C++17 (GCC, Clang ou MSVC).
* **Zip/Unzip/Tar/Curl** : Nécessaires pour que vcpkg télécharge les dépendances.

### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install git cmake build-essential curl zip unzip tar
```

### Windows
Nous recommandons l'utilisation de **Visual Studio 2022** avec la charge de travail "Développement Desktop C++".

## 2. Récupération du Projet

Clonez le dépôt officiel (assurez-vous d'avoir les droits d'accès) :

```bash
git clone [https://github.com/votre-orga/R-Type.git](https://github.com/votre-orga/R-Type.git)
cd R-Type
```

## 3. Gestion des Dépendances (vcpkg)

Le projet utilise **vcpkg** pour gérer automatiquement les bibliothèques tierces (SDL2, Asio, etc.). Vous n'avez pas besoin de les installer manuellement.

Initialisez vcpkg avec le script fourni :

```bash
# Sur Linux / macOS
git clone [https://github.com/microsoft/vcpkg.git](https://github.com/microsoft/vcpkg.git)
./vcpkg/bootstrap-vcpkg.sh

# Sur Windows (PowerShell/CMD)
git clone [https://github.com/microsoft/vcpkg.git](https://github.com/microsoft/vcpkg.git)
.\vcpkg\bootstrap-vcpkg.bat
```

## 4. Compilation

Utilisez CMake pour configurer et compiler le projet. L'option `-DCMAKE_TOOLCHAIN_FILE` est cruciale pour lier vcpkg.

### Linux / macOS
```bash
# Configuration (Release pour la performance)
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake

# Compilation
cmake --build build --config Release
```

### Windows
```bash
# Configuration
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=.\vcpkg\scripts\buildsystems\vcpkg.cmake

# Compilation
cmake --build build --config Release
```

Une fois terminé, les exécutables se trouvent dans le dossier `build/` (ou `build/Release/` sur Windows).