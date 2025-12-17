# Architecture du Moteur (ECS)

Le cœur du moteur de jeu R-Type repose sur une architecture **Entity Component System (ECS)**. Cette approche permet de découpler les données (Components) de la logique (Systems), offrant une meilleure performance et une grande modularité par rapport à l'héritage classique (OOP).

## Concepts Fondamentaux

### 1. Registry (Le Chef d'Orchestre)
Le `Registry` est la classe centrale qui gère tout. Il coordonne les entités et les composants. C'est l'unique point d'accès pour créer des objets ou récupérer des données.

* **Fichiers :** `src/common/ecs/registry.hpp`, `src/common/ecs/registry.cpp`

### 2. Entities (Les Identifiants)
Une Entité n'est rien d'autre qu'un identifiant unique (`uint32_t`). Elle ne contient ni données ni méthodes. Elle sert de clé pour associer différents composants entre eux via l'EntityManager.

* **Type :** `using Entity = uint32_t;`
* **Gestion :** `EntityManager` recycle les IDs des entités détruites pour économiser la mémoire.

### 3. Components (Les Données)
Un Composant est une structure de données pure (POD - Plain Old Data). Il ne doit contenir **aucune logique complexe**.

* **Stockage :** Les composants sont stockés dans des `ComponentArray<T>` (basés sur des `std::unordered_map`), ce qui permet un accès rapide via l'ID de l'entité.
* **Exemples existants :** `Position`, `Velocity`, `Sprite`.

### 4. Systems (La Logique)
Un Système contient la logique du jeu. Il itère sur les entités qui possèdent une signature spécifique (ex: toutes les entités ayant `Position` ET `Velocity`) pour effectuer des mises à jour.

---

## Guide d'Utilisation de l'API

Voici les méthodes principales du `Registry` pour manipuler le monde du jeu.

### Gestion des Entités

```cpp
// Créer une nouvelle entité
Entity player = reg.createEntity();

// Détruire une entité (et tous ses composants associés)
reg.destroyEntity(player);
```

### Manipulation des Composants

```cpp
// Ajouter un composant
reg.addComponent<Position>(player, 100.f, 200.f);
reg.addComponent<Velocity>(player, 10.f, 0.f);

// Vérifier si une entité a un composant
if (reg.hasComponent<Stats>(entity)) {
    // ...
}

// Récupérer un composant (Référence modifiable)
auto &pos = reg.getComponent<Position>(player);
pos.x += 5.0f;
```

### Itération (Views)

Pour traiter les entités par lot, utilisez `viewEntitiesWith`. Cette méthode est optimisée pour trouver l'intersection des entités possédant *tous* les composants demandés.

```cpp
// Récupère toutes les entités qui ont À LA FOIS Position et Velocity
auto view = reg.viewEntitiesWith<Position, Velocity>();

for (auto entity : view) {
    auto &pos = reg.getComponent<Position>(entity);
    auto &vel = reg.getComponent<Velocity>(entity);

    // Appliquer la logique
    pos.x += vel.vx * dt;
    pos.y += vel.vy * dt;
}
```

---

## Ajouter une Nouvelle Feature

Pour ajouter une fonctionnalité au jeu (ex: une "Hitbox"), suivez ces étapes strictes pour respecter l'architecture du projet.

### Étape 1 : Créer le Composant
Définissez la structure de données dans `src/common/ecs/components/`.

```cpp
// src/common/ecs/components/hitbox.hpp
struct Hitbox {
    float width;
    float height;
    bool active = true;
};
```

### Étape 2 : Enregistrer le Template (Crucial)
Le moteur utilise des templates explicites pour la compilation séparée. Vous **devez** enregistrer votre composant à la fin de `src/common/ecs/registry.cpp`, sinon vous aurez des erreurs de liens (linker errors).

Ajoutez ces lignes à la fin de `registry.cpp` :

```cpp
// Dans les instanciations de ensure
template std::shared_ptr<ComponentArray<Hitbox>> Registry::ensure<Hitbox>();

// Dans les instanciations de componentArray
template std::shared_ptr<ComponentArray<Hitbox>> Registry::componentArray<Hitbox>() const;

// Dans les instanciations de addComponent
template void Registry::addComponent<Hitbox, float, float, bool>(Entity, float, float, bool);

// Et pour les autres méthodes (hasComponent, getComponent, etc.) si nécessaire
```

### Étape 3 : Créer le Système
Implémentez la logique dans `src/common/ecs/systems/`.

```cpp
// src/common/ecs/systems/collision_system.cpp
void CollisionSystem::update(Registry& reg) {
    auto entities = reg.viewEntitiesWith<Position, Hitbox>();
    
    for (auto entity : entities) {
        // Logique de collision...
    }
}
```

### Étape 4 : Intégration
Instanciez votre système dans le `GameHandler` (Serveur) ou le `main.cpp` (Client) et appelez sa méthode `update()` dans la boucle principale.