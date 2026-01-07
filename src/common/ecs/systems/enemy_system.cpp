#include "enemy_system.hpp"
#include "../components/enemy.hpp"
#include "../components/position.hpp"
#include "../components/velocity.hpp"
#include "../components/projectile.hpp"
#include <algorithm>

void EnemySystem::update(Registry& reg, float dt) {
    newProjectileEntitiesWithParent.clear();
    newEnemyEntities.clear();
    projectileColliding.clear();
    deadEnemyEntities.clear();
    
    // Existing enemy logic (shooting, movement limits, collision)
    for (auto e : reg.viewEntitiesWith<Enemy, Position, Velocity>()) {
        Position& position = reg.getComponent<Position>(e);
        Velocity& velocity = reg.getComponent<Velocity>(e);
        Enemy& enemy = reg.getComponent<Enemy>(e);
        
        if (position.y > enemy.y_max_position) {
            velocity.vy = 0.f;
        }

        // Horizontal Movement Logic (ZigZag / Sine Wave)
        if (enemy.move_amplitude > 0.0f) {
            enemy.time_alive += dt;
            // v_x = A * w * cos(w * t)
            // A = amplitude, w = frequency * 2 * PI (if freq is Hz) or just frequency if simple scaler
            // Let's assume frequency is rad/s or scaler.
            // If user puts "1.0", they might expect 1 cycle per second? 
            // Let's stick to simple scaler first.
            velocity.vx = enemy.move_amplitude * enemy.move_frequency * cos(enemy.move_frequency * enemy.time_alive);
        }
        
        // Shoot logic
        if (enemy.canAttack()) {
            Entity projectile = reg.createEntity();
            reg.addComponent<Position>(projectile, position.x - 25.f, position.y - 50.f);
            reg.addComponent<Velocity>(projectile, 0.f, 200.f);
            reg.addComponent<Projectile>(projectile, enemy.damage, (std::string)"enemy");
            newProjectileEntitiesWithParent.push_back(std::make_pair(e, projectile));
            enemy.timeSinceLastShot = 0.0f;
        } else {
            enemy.timeSinceLastShot += dt;
        }
        
        // Collision logic
        for (auto& entity : reg.viewEntitiesWith<Projectile, Position, Velocity>()) {
            if (reg.getComponent<Projectile>(entity).ownerType == "player") {
                Position& projPos = reg.getComponent<Position>(entity);
                if (projPos.x >= position.x && projPos.x <= position.x + 50 &&
                    projPos.y >= position.y && projPos.y <= position.y + 50) {
                    
                    enemy.health -= reg.getComponent<Projectile>(entity).damage;
                    projectileColliding.push_back(entity);
                    
                    if (enemy.health <= 0) {
                        deadEnemyEntities.push_back(e);
                        enemyEntities.erase(std::remove(enemyEntities.begin(), enemyEntities.end(), e), enemyEntities.end());
                        reg.destroyEntity(e);
                        enemiesAlive--;
                    }
                }
            }
        }
    }
    
    // Spawn Logic
    if (!_levels.empty()) {
        if (_currentLevelIndex < (int)_levels.size()) {
            LevelData& level = _levels[_currentLevelIndex];
            
            // Check if we have waves
            if (_currentWaveIndex < (int)level.waves.size()) {
                WaveData& wave = level.waves[_currentWaveIndex];
                
                _waveTime += dt;
                
                if (_waveTime >= wave.start_delay) {
                    // Process groups
                    if (_currentGroupIndex < (int)wave.groups.size()) {
                        EnemyWaveGroupData& group = wave.groups[_currentGroupIndex];
                        _groupTimer += dt;
                        
                        // Spawn enemies in group
                        if (_spawnsInCurrentGroup < group.count) {
                            if (_groupTimer >= group.interval) {
                                _groupTimer = 0.0f;
                                
                                // Spawn One Enemy
                                if (_enemyTypes.find(group.type) != _enemyTypes.end()) {
                                    const auto& typeData = _enemyTypes[group.type];
                                    
                                    Entity enemyEntity = reg.createEntity();

                                    float spawnX;
                                    float amplitude = typeData.move_amplitude;
                                    float width = typeData.width;
                                    float screenWidth = 1080.0f;
                                    
                                    // Calculate safe spawn range so enemies don't go off screen
                                    float minX = amplitude + 10.0f;
                                    float maxX = screenWidth - amplitude - width - 10.0f;
                                    
                                    if (minX < maxX) {
                                        spawnX = minX + static_cast<float>(rand() % static_cast<int>(maxX - minX));
                                    } else {
                                        spawnX = (screenWidth - width) / 2.0f; // Center if amplitude is too big
                                    }

                                    float spawnY = -60.0f;
                                    
                                    reg.addComponent<Position>(enemyEntity, spawnX, spawnY);
                                    reg.addComponent<Velocity>(enemyEntity, typeData.velocity_x, typeData.velocity_y);
                                    // Note: Enemy Component constructor usage: type, health, damage, speed, fire_rate, lastShot, y_max
                                    reg.addComponent<Enemy>(enemyEntity, 
                                        typeData.name, 
                                        typeData.health, 
                                        typeData.damage, 
                                        static_cast<float>(typeData.speed), 
                                        2.0f, // fire rate hardcoded for now
                                        0.0f, 
                                        (int)(50 + rand() % 350), // Random stop position
                                        // New movement parameters
                                        typeData.move_amplitude,
                                        typeData.move_frequency,
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
                        // Check if all enemies dead? Or just proceed?
                        // If we want "clear wave to proceed":
                        if (enemiesAlive <= 0) {
                             _currentWaveIndex++;
                             _currentGroupIndex = 0;
                             _waveTime = 0.0f;
                        }
                    }
                }
            } else {
                // all waves done
                _currentLevelIndex++;
                _currentWaveIndex = 0;
                // Maybe loop or end?
            }
        }
    } else if (enemiesAlive <= 0 && canSpawn && _spawningEnabled) {
        // Legacy Wave System
        wave++;
        enemiesAlive = wave;
        for (int i = 0; i < enemiesAlive; ++i) {
            Entity enemyEntity = reg.createEntity();
            reg.addComponent<Position>(enemyEntity, static_cast<float>(rand() % 800), -50.f);
            reg.addComponent<Velocity>(enemyEntity, 0.f, 50.f);
            // Legacy fallback needs to match new constructor signature
            reg.addComponent<Enemy>(enemyEntity, (std::string)"basic", 100, 10, 50.f, 3.0f, 0.0f, (50 + rand() % 150), 0.0f, 0.0f, 0.0f);
            enemyEntities.push_back(enemyEntity);
            newEnemyEntities.push_back(enemyEntity);
        }
    }
}
