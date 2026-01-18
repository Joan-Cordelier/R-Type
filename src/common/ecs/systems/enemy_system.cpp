#include "enemy_system.hpp"
#include "../components/enemy.hpp"
#include "../components/position.hpp"
#include "../components/projectile.hpp"
#include "../components/stats.hpp"
#include "../components/velocity.hpp"
#include <algorithm>
#include <cmath>

void EnemySystem::update(Registry &reg, float dt) {
    newProjectileEntitiesWithParent.clear();
    newEnemyEntities.clear();
    projectileColliding.clear();
    deadEnemyEntities.clear();

    for (auto e : reg.viewEntitiesWith<Enemy, Position, Velocity>()) {
        if (_bossActive && e == _currentBossEntity)
            continue;

        Enemy &enemy = reg.getComponent<Enemy>(e);

        if (enemy.type == "void_zone") {
            enemy.time_alive += dt;

            float warningTime = 1.5f;

            if (enemy.time_alive < warningTime) {
                continue;
            }

            Position &pos = reg.getComponent<Position>(e);

            for (auto playerEntity : reg.viewEntitiesWith<Stats, Position>()) {
                Position &pPos = reg.getComponent<Position>(playerEntity);
                float dx = pPos.x - pos.x;
                float dy = pPos.y - pos.y;
                float distSq = dx * dx + dy * dy;

                if (distSq < 2500) {                       // Radius 50
                    if (enemy.timeSinceLastShot >= 0.5f) { // Tick every 0.5s
                        auto &stats = reg.getComponent<Stats>(playerEntity);
                        stats.hp -= enemy.damage;
                        enemy.timeSinceLastShot = 0.0f;

                        if (_statsUpdateCallback) {
                            _statsUpdateCallback(playerEntity);
                        }
                    } else {
                        enemy.timeSinceLastShot += dt;
                    }
                }
            }

            // Also lifetime?
            float duration = (enemy.max_lifetime > 0.0f) ? enemy.max_lifetime : 5.0f;
            if (enemy.time_alive > duration + warningTime) {
                deadEnemyEntities.push_back(e);
                // Deferred destruction
            }

            continue;
        }

        Position &position = reg.getComponent<Position>(e);
        Velocity &velocity = reg.getComponent<Velocity>(e);
        if (position.y > enemy.y_max_position) {
            velocity.vy = 0.f;
        }

        if (enemy.move_amplitude > 0.0f) {
            enemy.time_alive += dt;
            velocity.vx = enemy.move_amplitude * enemy.move_frequency *
                          cos(enemy.move_frequency * enemy.time_alive);
        }

        // Shoot logic
        if (enemy.canAttack()) {
            Entity projectile = reg.createEntity();
            reg.addComponent<Position>(projectile, position.x + _projectileOffsetX,
                                       position.y + _projectileOffsetY);
            reg.addComponent<Velocity>(projectile, 0.f, 200.f);
            reg.addComponent<Projectile>(projectile, enemy.damage, (std::string) "enemy");
            newProjectileEntitiesWithParent.push_back(std::make_pair(e, projectile));
            enemy.timeSinceLastShot = 0.0f;
        } else {
            enemy.timeSinceLastShot += dt;
        }

        // Collision logic
        bool isDead = false;

        // Disable collision for void zones (they are just damaging fields, shouldn't block shots)
        if (enemy.type != "void_zone") {
            for (auto &entity : reg.viewEntitiesWith<Projectile, Position, Velocity>()) {
                if (isDead)
                    break;
                if (reg.getComponent<Projectile>(entity).ownerType == "player") {
                    Position &projPos = reg.getComponent<Position>(entity);
                    if (projPos.x >= position.x && projPos.x <= position.x + 50 &&
                        projPos.y >= position.y && projPos.y <= position.y + 50) {

                        enemy.health -= reg.getComponent<Projectile>(entity).damage;
                        projectileColliding.push_back(entity);

                        if (enemy.health <= 0) {
                            deadEnemyEntities.push_back(e);
                            enemyEntities.erase(
                                std::remove(enemyEntities.begin(), enemyEntities.end(), e),
                                enemyEntities.end());
                            enemiesAlive--;
                            isDead = true;
                        }
                    }
                }
            }
        }
    }

    // Cleanup dead entities - remove duplicates but DON'T destroy them here
    // Let the caller (GameHandler) destroy them after processing score
    std::sort(deadEnemyEntities.begin(), deadEnemyEntities.end());
    deadEnemyEntities.erase(std::unique(deadEnemyEntities.begin(), deadEnemyEntities.end()),
                            deadEnemyEntities.end());

    // Spawn Logic
    if (canSpawn && !_levels.empty()) {
        if (_bossActive) {
            updateBoss(reg, dt);

            // Check if boss died
            if (std::find(enemyEntities.begin(), enemyEntities.end(), _currentBossEntity) ==
                enemyEntities.end()) {
                // Boss defeated
                _bossActive = false;
                _currentWaveIndex++;
                _currentGroupIndex = 0;
                _waveTime = 0.0f;
                _waveFinished = true;
            }
        } else if (_currentLevelIndex < (int)_levels.size()) {
            LevelData &level = _levels[_currentLevelIndex];

            // Check if we have waves
            if (_currentWaveIndex < (int)level.waves.size()) {
                WaveData &wave = level.waves[_currentWaveIndex];

                if (!_isWavePaused) {
                    _waveTime += dt;

                    if (_waveTime >= wave.start_delay) {
                        // Process groups
                        if (_currentGroupIndex < (int)wave.groups.size()) {
                            EnemyWaveGroupData &group = wave.groups[_currentGroupIndex];
                            _groupTimer += dt;

                            // Spawn enemies in group
                            if (_spawnsInCurrentGroup < group.count) {
                                if (_groupTimer >= group.interval) {
                                    _groupTimer = 0.0f;

                                    // Spawn One Enemy
                                    if (_enemyTypes.find(group.type) != _enemyTypes.end()) {
                                        const auto &typeData = _enemyTypes[group.type];

                                        Entity enemyEntity = reg.createEntity();

                                        float spawnX;
                                        float amplitude = typeData.move_amplitude;
                                        float width = typeData.width;
                                        float screenWidth = 1080.0f;

                                        // Calculate safe spawn range so enemies don't go off screen
                                        float minX = amplitude + 10.0f;
                                        float maxX = screenWidth - amplitude - width - 10.0f;

                                        if (minX < maxX) {
                                            spawnX =
                                                minX + static_cast<float>(
                                                           rand() % static_cast<int>(maxX - minX));
                                        } else {
                                            spawnX = (screenWidth - width) /
                                                     2.0f; // Center if amplitude is too big
                                        }

                                        float spawnY = -60.0f;

                                        reg.addComponent<Position>(enemyEntity, spawnX, spawnY);
                                        reg.addComponent<Velocity>(enemyEntity, typeData.velocity_x,
                                                                   typeData.velocity_y);
                                        // Note: Enemy Component constructor usage: type, health,
                                        // damage, speed, fire_rate, lastShot, y_max
                                        reg.addComponent<Enemy>(
                                            enemyEntity, typeData.name,
                                            static_cast<int>(typeData.health *
                                                             _difficultyMultiplier),
                                            typeData.damage, static_cast<float>(typeData.speed),
                                            2.0f, // fire rate hardcoded for now
                                            0.0f,
                                            (int)(50 + rand() % 350), // Random stop position
                                            // New movement parameters
                                            typeData.move_amplitude, typeData.move_frequency,
                                            0.0f // time_alive
                                        );

                                        enemyEntities.push_back(enemyEntity);
                                        newEnemyEntities.push_back(enemyEntity);
                                        enemiesAlive++;
                                    }

                                    _spawnsInCurrentGroup++;
                                }
                            } else {
                                // Group finished
                                _currentGroupIndex++;
                                _spawnsInCurrentGroup = 0;
                                _groupTimer = 0.0f;
                            }
                        } else {
                            // All groups spawned.
                            if (enemiesAlive <= 0) {
                                // Check for Boss
                                if (!wave.boss_id.empty()) {
                                    spawnBoss(reg, wave.boss_id);
                                } else {
                                    _currentWaveIndex++;
                                    _currentGroupIndex = 0;
                                    _waveTime = 0.0f;
                                    _waveFinished = true;
                                }
                            }
                        }
                    }
                }
            } else {
                // all waves done
                _currentLevelIndex++;
                _currentWaveIndex = 0;

                // Check if all levels are complete
                if (_currentLevelIndex >= (int)_levels.size()) {
                    if (_gameMode == GameMode::CLASSIC || _gameMode == GameMode::BOSS_RUSH ||
                        _gameMode == GameMode::SPEEDY) {
                        // Victory! All levels complete
                        if (!_gameComplete) {
                            _gameComplete = true;
                            if (_onGameVictory) {
                                _onGameVictory();
                            }
                        }
                    } else if (_gameMode == GameMode::ENDLESS) {
                        // Loop back to first level with increased difficulty
                        _currentLevelIndex = 0;
                        _endlessMultiplier++;
                        _difficultyMultiplier *= 1.2f; // Increase difficulty each loop
                    }
                }
            }
        }
    }
}

void EnemySystem::spawnBoss(Registry &reg, const std::string &bossId) {
    if (_bosses.find(bossId) == _bosses.end()) {
        // Boss config not found
        return;
    }

    // Reset state
    _bossActive = true;
    _currentBossId = bossId;
    _currentBossPhaseIndex = 0;
    _bossStateTimer = 0.0f;
    _currentPatternIndex = 0;
    _patternTimer = 0.0f;

    const auto &config = _bosses[bossId];

    _currentBossEntity = reg.createEntity();

    float startX = 1080.0f / 2.0f - (config.visuals.width / 2.0f); // Center
    float startY = -200.0f;                                        // Above screen

    reg.addComponent<Position>(_currentBossEntity, startX, startY);
    reg.addComponent<Velocity>(_currentBossEntity, 0.0f, 50.0f); // Move down slowly initially

    int initialHealth = static_cast<int>(config.health * _difficultyMultiplier);
    if (_playerCount > 1) {
        initialHealth +=
            static_cast<int>(config.health_per_player * _difficultyMultiplier * (_playerCount - 1));
    }

    // We reuse Enemy component but hijack it for boss stats
    reg.addComponent<Enemy>(_currentBossEntity, config.name, initialHealth,
                            80, // Collision damage
                            config.speed, 1.0f, 0.0f,
                            100, // Stop Y
                            0.0f, 0.0f, 0.0f);

    enemyEntities.push_back(_currentBossEntity);
    newEnemyEntities.push_back(_currentBossEntity); // To sync with clients
    enemiesAlive++;
}

void EnemySystem::updateBoss(Registry &reg, float dt) {
    if (!reg.hasComponent<Enemy>(_currentBossEntity)) {
        _bossActive = false;
        return;
    }

    // auto& enemyState = reg.getComponent<Enemy>(_currentBossEntity); // Unused for now
    auto &pos = reg.getComponent<Position>(_currentBossEntity);
    auto &vel = reg.getComponent<Velocity>(_currentBossEntity);

    if (_bosses.find(_currentBossId) == _bosses.end())
        return;
    const auto &config = _bosses.at(_currentBossId);

    // Phase Management
    auto &enemyState = reg.getComponent<Enemy>(_currentBossEntity);

    int scaledMaxHealth = static_cast<int>(config.max_health * _difficultyMultiplier);
    if (_playerCount > 1) {
        scaledMaxHealth +=
            static_cast<int>(config.health_per_player * _difficultyMultiplier * (_playerCount - 1));
    }
    float hpPercent = (float)enemyState.health / (float)scaledMaxHealth * 100.0f;

    // Boss Collision Logic
    float bossW = config.visuals.collider_width;
    float bossH = config.visuals.collider_height;

    for (auto &projEntity : reg.viewEntitiesWith<Projectile, Position>()) {
        auto &proj = reg.getComponent<Projectile>(projEntity);
        if (proj.ownerType == "player") {
            auto &projPos = reg.getComponent<Position>(projEntity);

            // AABB Collision
            if (projPos.x < pos.x + bossW &&
                projPos.x + 16 > pos.x && // 16 is approx projectile size
                projPos.y < pos.y + bossH && projPos.y + 16 > pos.y) {

                // Hit!
                enemyState.health -= proj.damage;
                projectileColliding.push_back(projEntity);

                // Do NOT destroy immediately to avoid ID reuse within same frame causing race
                // conditions reg.destroyEntity(projEntity);

                // Boss Death Logic
                if (enemyState.health <= 0) {
                    deadEnemyEntities.push_back(_currentBossEntity);
                    enemyEntities.erase(
                        std::remove(enemyEntities.begin(), enemyEntities.end(), _currentBossEntity),
                        enemyEntities.end());
                    reg.destroyEntity(_currentBossEntity);
                    enemiesAlive--;
                    // Break loop as boss is dead
                    break;
                }
            }
        }
    }

    // If boss died during collision check, stop updating
    if (!reg.hasComponent<Enemy>(_currentBossEntity)) {
        _bossActive = false;
        return;
    }

    if (_currentBossPhaseIndex < (int)config.phases.size()) {
        if (_currentBossPhaseIndex + 1 < (int)config.phases.size()) {
            if (hpPercent <= config.phases[_currentBossPhaseIndex + 1].trigger_health_percentage) {
                _currentBossPhaseIndex++;
                _currentPatternIndex = 0;
                _patternTimer = 0.0f;
                _currentPatternExecuted = false;
            }
        }
    }

    // Simple state machine for spawning patterns

    // Movement: Stop at Y=100
    if (pos.y >= 100.0f && vel.vy > 0.0f) {
        vel.vy = 0.0f;
    }

    // Execute patterns
    if (_currentBossPhaseIndex < (int)config.phases.size()) {
        const auto &curPhase = config.phases[_currentBossPhaseIndex];
        if (!curPhase.patterns.empty()) {
            _patternTimer += dt;

            std::string patternId = curPhase.patterns[_currentPatternIndex];
            auto it = std::find_if(config.patterns.begin(), config.patterns.end(),
                                   [&](const BossAttackPattern &p) { return p.id == patternId; });

            if (it != config.patterns.end()) {
                const auto &pattern = *it;

                if (!_currentPatternExecuted) {
                    executePattern(reg, pattern, _currentBossEntity);
                    _currentPatternExecuted = true;
                }

                float dur = (pattern.duration > 0.0f) ? pattern.duration : 3.0f;

                if (_patternTimer >= dur) {
                    _patternTimer = 0.0f;
                    _currentPatternExecuted = false;
                    _currentPatternIndex++;

                    if (_currentPatternIndex >= (int)curPhase.patterns.size()) {
                        if (curPhase.loop_patterns) {
                            _currentPatternIndex = 0;
                        } else {
                            _currentPatternIndex = (int)curPhase.patterns.size() - 1;
                            if (!curPhase.loop_patterns)
                                _currentPatternExecuted = true;
                        }
                    }
                }
            }
        }
    }
}

void EnemySystem::executePattern(Registry &reg, const BossAttackPattern &pattern,
                                 Entity bossEntity) {
    Position bossPos = reg.getComponent<Position>(bossEntity);

    if (pattern.type == "projectile_ring") {
        int count = pattern.count > 0 ? pattern.count : 8;
        float speed = pattern.speed > 0 ? pattern.speed : 200.0f;
        std::string projId = pattern.projectile_id.empty() ? "boss_orb" : pattern.projectile_id;

        for (int i = 0; i < count; ++i) {
            Entity proj = reg.createEntity();
            // Spread projectile on X axis
            float randX = 10 + (rand() % 1060);
            reg.addComponent<Position>(proj, randX, bossPos.y + 100.f);

            // Fall straight down to match client prediction
            reg.addComponent<Velocity>(proj, 0.f, speed);

            int dmg = (pattern.damage > 0) ? (int)pattern.damage : 20;
            reg.addComponent<Projectile>(proj, dmg, projId);
            newProjectileEntitiesWithParent.push_back({bossEntity, proj});
        }
    } else if (pattern.type == "area_spawner") {
        int count = pattern.count > 0 ? pattern.count : 3;
        for (int i = 0; i < count; ++i) {
            Entity zone = reg.createEntity();
            int rx = 20 + (rand() % 1000);
            int ry = 40 + (rand() % 640);

            reg.addComponent<Position>(zone, (float)rx, (float)ry);
            reg.addComponent<Velocity>(zone, 0.f, 0.f);

            int hp = 5000;
            int dmg = (pattern.damage > 0) ? (int)pattern.damage : 10;
            float duration = (pattern.duration > 0.f) ? pattern.duration : 5.0f;

            // Use specific type so client can render it
            reg.addComponent<Enemy>(zone, (std::string) "void_zone", hp, dmg, 0.f, 0.f, 0.f, 0, 0.f,
                                    0.f, 0.f,
                                    duration // max_lifetime
            );

            newEnemyEntities.push_back(zone);
            enemyEntities.push_back(zone);
        }
    }
}
