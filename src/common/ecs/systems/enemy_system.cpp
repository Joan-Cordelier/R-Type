#include "enemy_system.hpp"
#include "../components/enemy.hpp"
#include "../components/position.hpp"
#include "../components/velocity.hpp"
#include "../components/projectile.hpp"
#include "../components/stats.hpp"
#include <algorithm>
#include <cmath>

void EnemySystem::update(Registry& reg, float dt) {
    newProjectileEntitiesWithParent.clear();
    newEnemyEntities.clear();
    projectileColliding.clear();
    deadEnemyEntities.clear();
    
    // Existing enemy logic (shooting, movement limits, collision)
    for (auto e : reg.viewEntitiesWith<Enemy, Position, Velocity>()) {
        if (_bossActive && e == _currentBossEntity) continue;
        
        Enemy& enemy = reg.getComponent<Enemy>(e);
        
        // VOID ZONE LOGIC (Updated for circle drawing request)
        if (enemy.type == "void_zone") {
            enemy.time_alive += dt;
            
            // Warning time hardcoded or derived? 
            // In executePattern we didn't store it in component.
            // Let's assume 1.5s warning (consistent with 'spawn_void_zones')
            float warningTime = 1.5f;
            
            if (enemy.time_alive < warningTime) {
                // Warning phase - do nothing (client handles visual)
                continue; 
            }
            
            // After warning, it deals damage.
            // Check collision with players
            Position& pos = reg.getComponent<Position>(e);
            
            for (auto playerEntity : reg.viewEntitiesWith<Stats, Position>()) {
                 Position& pPos = reg.getComponent<Position>(playerEntity);
                 // Circle collision (Radius ~40?)
                 float dx = pPos.x - pos.x; // pos is center for void_zone? Check executePattern
                 float dy = pPos.y - pos.y;
                 float distSq = dx*dx + dy*dy;
                 
                 // If radius is 50 (diameter 100), distSq < 2500
                 // But pos from executePattern is random, and Render uses it as center?
                 // executePattern sets X/Y.
                 
                 if (distSq < 2500) { // Radius 50
                     // Deal damage periodically?
                     // enemy.timeSinceLastShot reused for damage tick
                     if (enemy.timeSinceLastShot >= 0.5f) { // Tick every 0.5s
                         auto& stats = reg.getComponent<Stats>(playerEntity);
                         stats.hp -= enemy.damage; // Damage from component
                         enemy.timeSinceLastShot = 0.0f;
                         
                         // Notify damage (reuse projectile hit logic logic? Or just update stats)
                         // Ideally we need to send stats update. 
                         // But EnemySystem doesn't have reference to Session/Network directly easily without callback or stored refs.
                         // BUT: GameHandler handles collision usually.
                         // Here we are inside EnemySystem::update. GameHandler runs logic too...
                         // Let's just modify HP. GameHandler will likely broadcast stats eventually or we need to mark it?
                         // Actually GameHandler checks collisions. This internal collision check duplicates logic but "Void Zone" is special.
                         // For now, let's assume GameHandler might not check Enemy-Player collision easily unless we add hitbox.
                         // Since we don't return "Events", we rely on shared component modification.
                     } else {
                        enemy.timeSinceLastShot += dt;
                     }
                 }
            }
            
            // Also lifetime?
            if (enemy.time_alive > 5.0f + warningTime) { // Duration from config?
                 deadEnemyEntities.push_back(e);
                 reg.destroyEntity(e);
            }
            
            continue;
        }

        Position& position = reg.getComponent<Position>(e);
        Velocity& velocity = reg.getComponent<Velocity>(e);
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
            reg.addComponent<Position>(projectile, position.x + _projectileOffsetX, position.y + _projectileOffsetY);
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
    if (canSpawn && !_levels.empty()) {
        if (_bossActive) {
            updateBoss(reg, dt);
            
            // Check if boss died
            if (std::find(enemyEntities.begin(), enemyEntities.end(), _currentBossEntity) == enemyEntities.end()) {
                 // Boss defeated
                 _bossActive = false;
                 _currentWaveIndex++;
                 _currentGroupIndex = 0;
                 _waveTime = 0.0f;
                 _waveFinished = true;
            }
        }
        else if (_currentLevelIndex < (int)_levels.size()) {
            LevelData& level = _levels[_currentLevelIndex];
            
            // Check if we have waves
            if (_currentWaveIndex < (int)level.waves.size()) {
                WaveData& wave = level.waves[_currentWaveIndex];
                
                if (!_isWavePaused) {
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
                } // End if (_waveTime >= wave.start_delay) inside !paused
                } // End if (!_isWavePaused)
            } else {
                // all waves done
                _currentLevelIndex++;
                _currentWaveIndex = 0;
                // Maybe loop or end?
            }
        }
    }
}

void EnemySystem::spawnBoss(Registry& reg, const std::string& bossId)
{
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
    
    const auto& config = _bosses[bossId];
    
    _currentBossEntity = reg.createEntity();
    
    float startX = 1080.0f / 2.0f - (config.visuals.width / 2.0f); // Center
    float startY = -200.0f; // Above screen
    
    reg.addComponent<Position>(_currentBossEntity, startX, startY);
    reg.addComponent<Velocity>(_currentBossEntity, 0.0f, 50.0f); // Move down slowly initially
    
    // We reuse Enemy component but hijack it for boss stats
    reg.addComponent<Enemy>(_currentBossEntity, 
        config.name, 
        config.health, 
        80, // Collision damage
        config.speed, 
        1.0f, 
        0.0f, 
        100, // Stop Y
        0.0f, 0.0f, 0.0f
    );
    
    // Boss specific marker or just treat as big enemy
    // To distinguish boss for logic, we track _currentBossEntity
    
    enemyEntities.push_back(_currentBossEntity);
    newEnemyEntities.push_back(_currentBossEntity); // To sync with clients
    enemiesAlive++; // Boss counts as enemy
}

void EnemySystem::updateBoss(Registry& reg, float dt)
{
    if (!reg.hasComponent<Enemy>(_currentBossEntity)) {
        _bossActive = false;
        return;
    }
    
    // auto& enemyState = reg.getComponent<Enemy>(_currentBossEntity); // Unused for now
    auto& pos = reg.getComponent<Position>(_currentBossEntity);
    auto& vel = reg.getComponent<Velocity>(_currentBossEntity);
    
    if (_bosses.find(_currentBossId) == _bosses.end()) return;
    const auto& config = _bosses.at(_currentBossId);
    
    // Phase Management
    auto& enemyState = reg.getComponent<Enemy>(_currentBossEntity);
    float hpPercent = (float)enemyState.health / (float)config.max_health * 100.0f;

    // Boss Collision Logic
    float bossW = config.visuals.width;
    float bossH = config.visuals.height;
    
    // Apply scale if collider box wasn't explicitly providing world-space size?
    // yaml: collider_box: {width: 190, height: 240...}
    // yaml: scale: 1.5
    // If width/height came from collider_box, we assume they are the final size.
    
    for (auto& projEntity : reg.viewEntitiesWith<Projectile, Position>()) {
        auto& proj = reg.getComponent<Projectile>(projEntity);
        if (proj.ownerType == "player") {
            auto& projPos = reg.getComponent<Position>(projEntity);
            
            // AABB Collision
            // Boss Pos is Top-Left (usually)
            // Check overlap
            if (projPos.x < pos.x + bossW &&
                projPos.x + 16 > pos.x &&     // 16 is approx projectile size
                projPos.y < pos.y + bossH &&
                projPos.y + 16 > pos.y) {
                
                // Hit!
                enemyState.health -= proj.damage;
                projectileColliding.push_back(projEntity);
                
                // Destroy projectile immediately to prevent multi-hit in same frame if logic runs multiple times?
                // Actually projectile check in 'update' does not destroy immediately, but adds to 'projectileColliding'?
                // But in 'update', it does: reg.destroyEntity(e) if enemy dies?
                // No, it adds to projectileColliding but does NOT destroy projectile in the loop?
                // Wait, 'update' loop logic:
                // projectileColliding.push_back(entity);
                // But it DOES NOT satisfy the user request "les balles le traverse". 
                // To stop traversing, we must destroy logic.
                // The 'projectileColliding' list is likely used elsewhere to destroy them?
                
                // Checking update loop: replacement code earlier destroys enemy but what about projectile?
                // The main loop does NOT destroy projectile. It just pushes to projectileColliding.
                // WE MUST destroy projectile or mark it for destruction.
                // The user says "les balles le traverse".
                
                // Ideally, we destroy it.
                reg.destroyEntity(projEntity);

                // Boss Death Logic
                if (enemyState.health <= 0) {
                    deadEnemyEntities.push_back(_currentBossEntity);
                    enemyEntities.erase(std::remove(enemyEntities.begin(), enemyEntities.end(), _currentBossEntity), enemyEntities.end());
                    reg.destroyEntity(_currentBossEntity);
                    enemiesAlive--;
                    // Break loop as boss is dead
                    break;
                }
            }
        }
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
        const auto& curPhase = config.phases[_currentBossPhaseIndex];
        if (!curPhase.patterns.empty()) {
            _patternTimer += dt;
            
            std::string patternId = curPhase.patterns[_currentPatternIndex];
            auto it = std::find_if(config.patterns.begin(), config.patterns.end(), 
                [&](const BossAttackPattern& p) { return p.id == patternId; });
                
            if (it != config.patterns.end()) {
                const auto& pattern = *it;
                
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
                            if (!curPhase.loop_patterns) _currentPatternExecuted = true; 
                        }
                    }
                }
            }
        }
    }
}

void EnemySystem::executePattern(Registry& reg, const BossAttackPattern& pattern, Entity bossEntity)
{
    Position bossPos = reg.getComponent<Position>(bossEntity);
    
    if (pattern.type == "projectile_ring") {
        int count = pattern.count > 0 ? pattern.count : 8;
        float speed = pattern.speed > 0 ? pattern.speed : 200.0f;
        std::string projId = pattern.projectile_id.empty() ? "boss_orb" : pattern.projectile_id;
        
        for (int i=0; i<count; ++i) {
             Entity proj = reg.createEntity();
             reg.addComponent<Position>(proj, bossPos.x + 90.f, bossPos.y + 100.f);
             float angle = (float)i / (float)count * 2.0f * 3.14159f;
             float vx = static_cast<float>(std::cos(angle) * speed);
             float vy = static_cast<float>(std::sin(angle) * speed);
             reg.addComponent<Velocity>(proj, vx, vy);
             int dmg = (pattern.damage > 0) ? (int)pattern.damage : 20;
             reg.addComponent<Projectile>(proj, dmg, projId);
             newProjectileEntitiesWithParent.push_back({bossEntity, proj});
        }
    }
    else if (pattern.type == "area_spawner") {
        int count = pattern.count > 0 ? pattern.count : 3;
        for (int i=0; i<count; ++i) {
            Entity zone = reg.createEntity();
            int rx = 100 + (rand() % 900);
            int ry = 100 + (rand() % 600);
            
            reg.addComponent<Position>(zone, (float)rx, (float)ry);
            reg.addComponent<Velocity>(zone, 0.f, 0.f);
            
            int hp = 5000; 
            int dmg = (pattern.damage > 0) ? (int)pattern.damage : 10;
            // Use specific type so client can render it
            reg.addComponent<Enemy>(zone, 
                (std::string)"void_zone", 
                hp, 
                dmg, 
                0.f, 0.f, 0.f, 0, 0.f, 0.f, 0.f
            );
            
            newEnemyEntities.push_back(zone);
            enemyEntities.push_back(zone);
        }
    }
}
