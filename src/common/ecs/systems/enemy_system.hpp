#pragma once

#include "../registry.hpp"
#include "../../Data/LevelData.hpp"
#include <vector>
#include <random>
#include <map>

class EnemySystem {
    public:
        void update(Registry& reg, float dt);
        void setLevels(const std::vector<LevelData>& levels) { _levels = levels; }
        void setEnemyTypes(const std::map<std::string, EnemyTypeData>& types) { _enemyTypes = types; }

        std::vector<Entity>& getEnemyEntities() { return enemyEntities; }
        std::vector<std::pair<Entity, Entity>>& getNewProjectileEntitiesWithParent() { return newProjectileEntitiesWithParent; }
        std::vector<Entity>& getNewEnemyEntities() { return newEnemyEntities; }
        void startSpawning() { canSpawn = true; }
        std::vector<Entity>& getProjectileColliding() { return projectileColliding; }
        std::vector<Entity>& getDeadEnemyEntities() { return deadEnemyEntities; }
        
        void setSpawnInterval(float interval) { _spawnInterval = interval; }
        void setMaxEnemies(int max) { _maxEnemies = max; }
        void setInitialDelay(float delay) { _initialDelay = delay; }
        void disableSpawning() { _spawningEnabled = false; }
        void setProjectileOffsets(float offsetX, float offsetY) { 
            _projectileOffsetX = offsetX; 
            _projectileOffsetY = offsetY; 
        }
    private:
        std::vector<Entity> enemyEntities;
        std::vector<std::pair<Entity, Entity>> newProjectileEntitiesWithParent;
        std::vector<Entity> newEnemyEntities;
        std::vector<Entity> projectileColliding;
        std::vector<Entity> deadEnemyEntities;
        int enemiesAlive = 0;
        int wave = 0;
        bool canSpawn = false;
        
        float _spawnInterval = 3.0f;
        int _maxEnemies = 10;
        float _initialDelay = 2.0f;
        bool _spawningEnabled = true;
        float _projectileOffsetX = 18.0f;  // Default from config
        float _projectileOffsetY = 50.0f;

        // Level System Data
        std::vector<LevelData> _levels;
        std::map<std::string, EnemyTypeData> _enemyTypes;
        int _currentLevelIndex = 0;
        int _currentWaveIndex = 0;
        int _currentGroupIndex = 0;
        float _levelTime = 0.0f;
        float _waveTime = 0.0f;
        int _spawnsInCurrentGroup = 0;
        float _groupTimer = 0.0f; 
        bool _levelSystemActive = false;
};