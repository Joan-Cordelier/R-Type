# Guide : Ajouter une Feature (Gameplay)

Ce guide explique comment ajouter une nouvelle fonctionnalité de jeu en respectant l'architecture ECS (Entity Component System) du projet.

Pour l'exemple, nous allons implémenter un **Bouclier (Shield)** qui protège une entité.

## Résumé des étapes
1.  **Component** : Créer la structure de données.
2.  **Registry** : Enregistrer le composant (Templates).
3.  **System** : Créer la logique.
4.  **Intégration** : Ajouter le système dans la boucle de jeu.

---

## 1. Créer le Composant (Data)

Créez un nouveau fichier dans `src/common/ecs/components/shield.hpp`.
Un composant doit être une structure simple (POD) sans logique complexe.

```cpp
#ifndef ECS_COMPONENTS_SHIELD_HPP
#define ECS_COMPONENTS_SHIELD_HPP

struct Shield {
    int integrity = 100;    // Points de vie du bouclier
    bool active = true;     // État du bouclier
    float rechargeRate = 1.0f; 
};

#endif
```

## 2. Enregistrer le Composant (Registry)

C'est l'étape la plus souvent oubliée. Le moteur utilisant des templates explicites pour la compilation, vous devez déclarer votre composant dans `src/common/ecs/registry.cpp`.

Ouvrez `registry.cpp` et ajoutez ces lignes à la fin du fichier, avec les autres instanciations :

```cpp
#include "components/shield.hpp" // N'oubliez pas l'include en haut !

// ... (à la toute fin du fichier)

// Enregistrement du tableau de composants
template std::shared_ptr<ComponentArray<Shield>> Registry::ensure<Shield>();
template std::shared_ptr<ComponentArray<Shield>> Registry::componentArray<Shield>() const;

// Enregistrement de la méthode addComponent (précisez les types des arguments du constructeur de votre struct)
template void Registry::addComponent<Shield, int, bool, float>(Entity, int, bool, float);

// Enregistrement des getters/checkers
template bool Registry::hasComponent<Shield>(Entity) const;
template Shield& Registry::getComponent<Shield>(Entity);
```

## 3. Créer le Système (Logic)

Créez votre système dans `src/common/ecs/systems/shield_system.hpp` et `.cpp`.

**shield_system.hpp**
```cpp
class ShieldSystem {
public:
    void update(Registry& reg, float dt);
};
```

**shield_system.cpp**
```cpp
#include "shield_system.hpp"
#include "../components/shield.hpp"
#include "../components/position.hpp" // Si besoin de la position

void ShieldSystem::update(Registry& reg, float dt) {
    // On récupère toutes les entités qui ont un Bouclier
    auto view = reg.viewEntitiesWith<Shield>();

    for (auto entity : view) {
        auto &shield = reg.getComponent<Shield>(entity);

        if (shield.active && shield.integrity < 100) {
            // Logique : recharger le bouclier avec le temps
            shield.integrity += shield.rechargeRate * dt;
        }
    }
}
```

## 4. Intégration (Game Loop)

Enfin, instanciez votre système là où il doit tourner (généralement côté Serveur pour la logique autoritaire).

Dans `src/server/Gameplay/GameHandler.hpp` :
```cpp
#include "ecs/systems/shield_system.hpp"

class GameHandler {
    // ...
    ShieldSystem shieldSystem; // Ajouter le membre
};
```

Dans `src/server/Gameplay/GameHandler.cpp` (méthode `updateGame`) :
```cpp
void GameHandler::updateGame(float deltaTime)
{
    movement.update(reg, deltaTime);
    shieldSystem.update(reg, deltaTime); // <-- Ajouter l'appel ici
    // ...
}
```