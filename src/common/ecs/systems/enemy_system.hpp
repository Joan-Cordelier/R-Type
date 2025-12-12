#pragma once

#include "../registry.hpp"
#include <vector>
#include <random>

class EnemySystem {
    public:
        void update(Registry& reg, float dt);
        std::vector<Entity>& getEnemyEntities() { return enemyEntities; }
        std::vector<std::pair<Entity, Entity>>& getNewProjectileEntitiesWithParent() { return newProjectileEntitiesWithParent; }
        std::vector<Entity>& getNewEnemyEntities() { return newEnemyEntities; }
        void startSpawning() { canSpawn = true; }
        std::vector<Entity>& getProjectileColliding() { return projectileColliding; }
    private:
        std::vector<Entity> enemyEntities;
        std::vector<std::pair<Entity, Entity>> newProjectileEntitiesWithParent;
        std::vector<Entity> newEnemyEntities;
        std::vector<Entity> projectileColliding;
        int enemiesAlive = 0;
        int wave = 0;
        bool canSpawn = false;
};