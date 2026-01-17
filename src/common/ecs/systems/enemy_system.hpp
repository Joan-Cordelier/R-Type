#pragma once

#include "../registry.hpp"
#include "../../Data/LevelData.hpp"
#include "../../Config/GameLoopConfig.hpp"
#include <vector>
#include <random>
#include <map>
#include <functional>

class EnemySystem {
    public:
        void update(Registry& reg, float dt);
        void setLevels(const std::vector<LevelData>& levels) { _levels = levels; }
        void setEnemyTypes(const std::map<std::string, EnemyTypeData>& types) { _enemyTypes = types; }
        void setBosses(const std::map<std::string, BossConfig>& bosses) { _bosses = bosses; }

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
        
        bool isWaveFinished() const { return _waveFinished; }
        void acknowledgeWaveFinished() { _waveFinished = false; }
        
        void setWavePaused(bool paused) { _isWavePaused = paused; }

        void setStatsUpdateCallback(std::function<void(Entity)> callback) {
            _statsUpdateCallback = callback;
        }

        void setPlayerCount(int count) { _playerCount = count; }
        void setDifficultyMultiplier(float multiplier) { _difficultyMultiplier = multiplier; }

    private:
        void spawnBoss(Registry& reg, const std::string& bossId);
        void updateBoss(Registry& reg, float dt);
        void executePattern(Registry& reg, const BossAttackPattern& pattern, Entity bossEntity);

        std::function<void(Entity)> _statsUpdateCallback;
        std::vector<Entity> enemyEntities;
        std::vector<std::pair<Entity, Entity>> newProjectileEntitiesWithParent;
        std::vector<Entity> newEnemyEntities;
        std::vector<Entity> projectileColliding;
        std::vector<Entity> deadEnemyEntities;
        int enemiesAlive = 0;
        int wave = 0;
        bool canSpawn = false;
        int _playerCount = 1;
        float _difficultyMultiplier = 1.0f;
        
        float _spawnInterval = 3.0f;
        int _maxEnemies = 10;
        float _initialDelay = 2.0f;
        bool _spawningEnabled = true;
        float _projectileOffsetX = 18.0f;  // Default from config
        float _projectileOffsetY = 50.0f;

        // Level System Data
        std::vector<LevelData> _levels;
        std::map<std::string, EnemyTypeData> _enemyTypes;
        std::map<std::string, BossConfig> _bosses;
        int _currentLevelIndex = 0;
        
        // Boss Logic State
        bool _bossActive = false;
        Entity _currentBossEntity = 99999;
        std::string _currentBossId;
        int _currentBossPhaseIndex = 0;
        float _bossStateTimer = 0.0f;
        int _currentPatternIndex = 0;
        float _patternTimer = 0.0f;
        bool _currentPatternExecuted = false;
        
        bool _waveFinished = false;
        bool _isWavePaused = false;
        
        int _currentWaveIndex = 0;
        int _currentGroupIndex = 0;
        float _levelTime = 0.0f;
        float _waveTime = 0.0f;
        int _spawnsInCurrentGroup = 0;
        float _groupTimer = 0.0f; 
        bool _levelSystemActive = false;
};