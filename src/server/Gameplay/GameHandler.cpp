#include "GameHandler.hpp"
#include "../../common/Data/EntityType.hpp"
#include "../../common/ecs/components/parent.hpp"
#include "../Logs/Logger.hpp"

#include <algorithm>
#include <chrono>
#include <random>
#include <thread>

GameHandler::GameHandler(SessionManager &session,
                         std::shared_ptr<ThreadedQueue<DecodedMessage>> inputQueue,
                         std::atomic<bool> &running, const std::string &configPath,
                         PrometheusExporter &monitor, Difficulty difficulty, GameMode gameMode)
    : _running(running), _session(session), _monitor(monitor), _difficulty(difficulty),
      _gameMode(gameMode), _messageHandler(session, *inputQueue, running), _inputQueue(inputQueue) {
    // Load configuration
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        LOG_INFO("Server CWD: " + std::string(cwd));
    }

    if (!_config.loadFromFile(configPath)) {
        LOG_WARN("Failed to load config from " + configPath + ", trying ../" + configPath);
        if (!_config.loadFromFile("../" + configPath)) {
            LOG_WARN("Failed to load config from ../" + configPath + ", using defaults");
        } else {
            LOG_INFO("Successfully loaded config from ../" + configPath);
        }
    } else {
        LOG_INFO("Successfully loaded config from " + configPath);
    }

    // Set difficulty multiplier
    float difficultyMultiplier = 1.0f;
    switch (_difficulty) {
    case Difficulty::EASY:
        difficultyMultiplier = _config.getDifficultyConfig().easy;
        break;
    case Difficulty::NORMAL:
        difficultyMultiplier = _config.getDifficultyConfig().normal;
        break;
    case Difficulty::HARD:
        difficultyMultiplier = _config.getDifficultyConfig().hard;
        break;
    case Difficulty::IMPOSSIBLE:
        difficultyMultiplier = _config.getDifficultyConfig().impossible;
        break;
    default:
        difficultyMultiplier = 1.0f;
        break;
    }
    enemySystem.setDifficultyMultiplier(difficultyMultiplier);

    // Print verify loaded values
    auto &hb = _config.getPlayerConfig().hitbox;
    LOG_INFO("Loaded Player Hitbox: W=" + std::to_string(hb.width) +
             " H=" + std::to_string(hb.height) + " OffX=" + std::to_string(hb.offset_x) +
             " OffY=" + std::to_string(hb.offset_y));

    // Configure EnemySystem
    enemySystem.setSpawnInterval(_config.getEnemySpawning().spawn_interval);
    enemySystem.setMaxEnemies(_config.getEnemySpawning().max_enemies);
    enemySystem.setInitialDelay(_config.getEnemySpawning().initial_delay);
    if (!_config.getEnemySpawning().enabled) {
        enemySystem.disableSpawning();
    }

    // Set level data
    enemySystem.setLevels(_config.getLevels());
    enemySystem.setEnemyTypes(_config.getEnemyTypes());

    // Configure enemy projectile offsets from config
    auto &projConfig = _config.getProjectilesConfig().enemy;
    enemySystem.setProjectileOffsets(projConfig.offset_x, projConfig.offset_y);
    LOG_INFO("Enemy projectile offsets: X=" + std::to_string(projConfig.offset_x) +
             " Y=" + std::to_string(projConfig.offset_y));

    // Load upgrades configuration
    if (!_config.loadUpgradesFromFile("yaml/upgrades.yaml")) {
        if (!_config.loadUpgradesFromFile("../yaml/upgrades.yaml")) {
            LOG_WARN("Could not load upgrades from yaml/upgrades.yaml");
        } else {
            LOG_INFO("Loaded upgrades from ../yaml/upgrades.yaml");
        }
    } else {
        LOG_INFO("Loaded upgrades from yaml/upgrades.yaml");
    }

    // Load Bosses configuration
    std::string bossDir = "yaml/enemies";
    if (!_config.loadBossesFromDirectory(bossDir)) {
        if (!_config.loadBossesFromDirectory("../" + bossDir)) {
            LOG_WARN("Could not load bosses from " + bossDir);
        }
    }
    enemySystem.setBosses(_config.getBosses());

    // Configure game mode
    enemySystem.setGameMode(_gameMode);
    enemySystem.setOnGameVictory([this]() {
        LOG_INFO("=== GAME VICTORY - All levels complete! ===");
        if (_onGameVictory) {
            _onGameVictory(static_cast<uint32_t>(score));
        }
    });

    enemySystem.setStatsUpdateCallback([this](Entity playerEntity) {
        if (reg.hasComponent<Stats>(playerEntity)) {
            auto &stats = reg.getComponent<Stats>(playerEntity);
            MessageData statsPayload = MessageFactory::getInstance().encodeMessageUpdateStats(
                playerEntity, stats.hp, stats.maxHp, stats.movement_speed);
            PreparedMessage statsMsg =
                MessageFactory::getInstance().createMessage(OpCode::UPDATE_STATS, statsPayload);
            for (const auto &[pid, _] : playerEntities) {
                _session.sendTcp(pid, statsMsg);
            }

            // Note: Death handling is done in checkPlayerCollisions()
            // to avoid duplicate logic. The hp is already updated,
            // and checkPlayerCollisions() will detect hp <= 0.
        }
    });

    // Set up message handler callbacks
    _messageHandler.setOnPlayerConnect([this](const Player &player) { onPlayerConnect(player); });

    _messageHandler.setOnPlayerMove([this](const MoveData &moveData) { onPlayerMove(moveData); });

    _messageHandler.setOnPlayerShoot(
        [this](const ShootData &shootData) { onPlayerShoot(shootData); });

    _messageHandler.setOnPlayerLink([this](uint32_t playerId) { onPlayerLinked(playerId); });

    _messageHandler.setOnUpgradeSelect(
        [this](uint32_t playerId, uint8_t index) { onUpgradeSelect(playerId, index); });

    // Handle DISCONNECT messages from Room's queue (when player disconnects via TCP)
    _messageHandler.setOnPlayerDisconnect(
        [this](const Player &player) { onPlayerDisconnect(player.id); });

    ScoreEntity = reg.createEntity();
    reg.addComponent<Position>(ScoreEntity, 0.f, 0.f);
    reg.addComponent<Label>(ScoreEntity, std::string("Score: 0"),
                            std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                            std::string("default_font"), Color(255, 255, 255), 0, true);

    weaponSystem.setProjectileNotifier([this](Entity parent, Entity projectile) {
        sendNewProjectilesToAllPlayers(parent, projectile);
    });

    // Configure projectile offsets and speed from config
    weaponSystem.setProjectileConfig(_config.getProjectilesConfig().player.offset_x,
                                     _config.getProjectilesConfig().player.offset_y,
                                     _config.getProjectilesConfig().player.speed);
}

void GameHandler::run() {
    LOG_INFO("Game loop started");

    auto lastTime = std::chrono::steady_clock::now();
    constexpr float targetFrameTime = 1.0f / 30.0f; // 30 ticks per second

    while (_running) {
        auto loopStart = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Process all pending messages
        processMessages();

        // Update game state
        updateGame(deltaTime);

        // Send game packet to players
        sendUpdatedPositionToAllPlayers();

        auto loopEnd = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = loopEnd - loopStart;
        _monitor.updateGameLoopDuration(elapsed.count());

        // Frame rate limiting
        auto frameEnd = std::chrono::steady_clock::now();
        float frameTime = std::chrono::duration<float>(frameEnd - currentTime).count();
        if (frameTime < targetFrameTime) {
            std::this_thread::sleep_for(std::chrono::duration<float>(targetFrameTime - frameTime));
        }
    }

    LOG_INFO("Game loop stopped");
}

void GameHandler::sendUpdatedPositionToAllPlayers() {
    for (const auto &[playerId, entity] : playerEntities) {
        sendUpdatedPositionToPlayer(playerId);
    }
    for (const auto &[playerId, entity] : playerEntities) {
        wasMoving[entity] =
            reg.hasComponent<Velocity>(entity) && (reg.getComponent<Velocity>(entity).vx != 0.f ||
                                                   reg.getComponent<Velocity>(entity).vy != 0.f);
    }
}

void GameHandler::sendNewProjectilesToAllPlayers(Entity parentEntity, Entity projectileEntity) {
    std::string ownerType = "player";
    float scale = 1.0f;
    if (reg.hasComponent<Projectile>(projectileEntity)) {
        auto &proj = reg.getComponent<Projectile>(projectileEntity);
        ownerType = proj.ownerType;
        scale = proj.scale;
    }

    // Get projectile position and include it in the message
    float x = 0.f, y = 0.f;
    if (reg.hasComponent<Position>(projectileEntity)) {
        Position &pos = reg.getComponent<Position>(projectileEntity);
        x = pos.x;
        y = pos.y;
    }

    float vx = 0.f, vy = 0.f;
    if (reg.hasComponent<Velocity>(projectileEntity)) {
        Velocity &vel = reg.getComponent<Velocity>(projectileEntity);
        vx = vel.vx;
        vy = vel.vy;
    }

    MessageData payload = MessageFactory::getInstance().encodeMessageProjectile(
        projectileEntity, parentEntity, ownerType, x, y, vx, vy, scale);
    PreparedMessage msg = MessageFactory::getInstance().createMessage(OpCode::SHOOT, payload);

    for (const auto &[playerId, entity] : playerEntities) {
        _session.sendTcp(playerId, msg);
    }

    LOG_DEBUG("Sent projectile " + std::to_string(projectileEntity) + " from parent " +
              std::to_string(parentEntity) + " to all players at (" + std::to_string(x) + ", " +
              std::to_string(y) + ") scale:" + std::to_string(scale));
}

/// @brief send cur position of all entity containing a vector to a player
/// @param playerId the id of the player to send the position to
void GameHandler::sendUpdatedPositionToPlayer(uint32_t playerId) {
    auto &factory = MessageFactory::getInstance();

    // Send player positions
    for (const auto &[pid, entity] : playerEntities) {
        if (!reg.hasComponent<Position>(entity) || !reg.hasComponent<Velocity>(entity))
            continue;

        Position &pos = reg.getComponent<Position>(entity);

        bool isMoving = reg.getComponent<Velocity>(entity).vx != 0.f ||
                        reg.getComponent<Velocity>(entity).vy != 0.f;
        bool entityWasMoving = wasMoving[entity];

        if (isMoving || entityWasMoving) {
            MessageData payload =
                factory.encodeMessageMove(EntityType::PLAYER, entity, pos.x, pos.y);
            PreparedMessage msg =
                factory.createMessage(OpCode::MOVE_SYNC, payload); // <-- MOVE_SYNC
            _session.sendUdp(playerId, msg);
        }
    }

    // Send companion positions
    for (auto entity : reg.viewEntitiesWith<Parent, Position>()) {
        if (!reg.hasComponent<Parent>(entity))
            continue;
        Position &pos = reg.getComponent<Position>(entity);
        MessageData payload =
            factory.encodeMessageMove(EntityType::COMPANION, entity, pos.x, pos.y);
        PreparedMessage msg = factory.createMessage(OpCode::MOVE_SYNC, payload);
        _session.sendUdp(playerId, msg);
    }
}

void GameHandler::processMessages() {
    // Process up to a maximum number of messages per frame to avoid starvation
    constexpr int maxMessagesPerFrame = 100;
    int processed = 0;

    while (processed < maxMessagesPerFrame && _messageHandler.processSingleCycle()) {
        processed++;
    }
}

void GameHandler::updateGame(float deltaTime) {
    if (_waitingForUpgrades) {
        return;
    }

    // Update companions positions
    for (auto entity : reg.viewEntitiesWith<Parent, Position>()) {
        if (!reg.hasComponent<Parent>(entity))
            continue;
        auto &parent = reg.getComponent<Parent>(entity);
        auto &pos = reg.getComponent<Position>(entity);

        if (reg.hasComponent<Position>(parent.entity)) {
            auto &parentPos = reg.getComponent<Position>(parent.entity);
            pos.x = parentPos.x + parent.offsetX;
            pos.y = parentPos.y + parent.offsetY;
        }
    }

    enemySystem.setPlayerCount(playerEntities.size());
    enemySystem.update(reg, deltaTime);

    // Check for wave completion to trigger upgrades
    if (enemySystem.isWaveFinished()) {
        enemySystem.acknowledgeWaveFinished();
        enemySystem.setWavePaused(true);
        _waitingForUpgrades = true;
        _playersSelectedUpgrade.clear();

        LOG_INFO("Wave Finished! Select an upgrade:");

        const auto &allUpgrades = _config.getUpgrades();
        if (allUpgrades.size() >= 3) {
            // Pick 3 random upgrades
            std::vector<UpgradeData> candidates = allUpgrades;
            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(candidates.begin(), candidates.end(), g);

            _offeredUpgrades.clear();
            std::vector<std::string> upgradeIds;
            for (size_t i = 0; i < 3 && i < candidates.size(); ++i) {
                _offeredUpgrades.push_back(candidates[i]);
                upgradeIds.push_back(candidates[i].id);
                LOG_INFO("[" + std::to_string(i) + "] " + candidates[i].name + " " +
                         candidates[i].id);
            }

            auto &factory = MessageFactory::getInstance();
            MessageData payload = factory.encodeMessageUpgradeOptions(upgradeIds);
            PreparedMessage msg = factory.createMessage(OpCode::UPGRADE_OPTIONS, payload);

            for (const auto &[playerId, entity] : playerEntities) {
                // Using TCP for reliability of the menu opening
                _session.sendTcp(playerId, msg);
            }
            LOG_INFO("Sent upgrade options to all players.");

        } else {
            LOG_WARN("Not enough upgrades configured. Skipping phase.");
            _waitingForUpgrades = false;
            enemySystem.setWavePaused(false);
        }
    }

    for (const auto &enemy : enemySystem.getDeadEnemyEntities()) {
        // Add score for killed enemy
        if (reg.hasComponent<Enemy>(enemy)) {
            auto &enemyComp = reg.getComponent<Enemy>(enemy);
            // Look up score from config by enemy type
            const auto &enemyTypes = _config.getEnemyTypes();
            auto it = enemyTypes.find(enemyComp.type);
            if (it != enemyTypes.end()) {
                score += it->second.score;
                LOG_INFO("Enemy '" + enemyComp.type + "' killed! Score +" +
                         std::to_string(it->second.score) + " (Total: " + std::to_string(score) +
                         ")");
                sendScoreUpdateToAllPlayers();
            } else {
                // Default score if type not found
                score += 100;
                LOG_WARN("Enemy type '" + enemyComp.type +
                         "' not found in config, using default score 100 (Total: " +
                         std::to_string(score) + ")");
                sendScoreUpdateToAllPlayers();
            }
        } else {
            LOG_WARN("Dead enemy entity has no Enemy component");
        }
        sendDestroyedEnemyToAllPlayers(enemy);
        // Destroy the enemy entity after processing
        reg.destroyEntity(enemy);
    }
    for (const auto &projectile : enemySystem.getProjectileColliding()) {
        sendDestroyedProjectileToAllPlayers(projectile);
        reg.destroyEntity(projectile);
    }

    for (const auto &[enemy, projectile] : enemySystem.getNewProjectileEntitiesWithParent()) {
        sendNewProjectilesToAllPlayers(enemy, projectile);
    }

    initNewEnemyEntities(enemySystem.getNewEnemyEntities());
    updateEnemyPosition(enemySystem.getEnemyEntities());
    movement.update(reg, deltaTime);
    statsys.update(reg, deltaTime);

    // Auto-fire companions
    for (auto entity : reg.viewEntitiesWith<Parent, Weapon>()) {
        weaponSystem.fireWeapon(reg, entity);
    }

    // Auto-fire players
    for (const auto &[playerId, entity] : playerEntities) {
        if (reg.hasComponent<Weapon>(entity)) {
            weaponSystem.fireWeapon(reg, entity);
        }
    }

    weaponSystem.update(reg, deltaTime);

    checkPlayerCollisions();
}

void GameHandler::sendDestroyedProjectileToAllPlayers(Entity projectile) {
    auto &factory = MessageFactory::getInstance();
    MessageData payload = factory.encodeMessageDeath(EntityType::PROJECTILE, projectile);
    PreparedMessage msg = factory.createMessage(OpCode::DEATH, payload);

    for (const auto &[playerId, entity] : playerEntities) {
        _session.sendTcp(playerId, msg);
    }

    LOG_DEBUG("Sent DEATH for projectile " + std::to_string(projectile) + " to all players");
}

void GameHandler::sendDestroyedEnemyToAllPlayers(Entity enemy) {
    auto &factory = MessageFactory::getInstance();
    MessageData payload = factory.encodeMessageDeath(EntityType::ENEMY, enemy);
    PreparedMessage msg = factory.createMessage(OpCode::DEATH, payload);

    for (const auto &[playerId, entity] : playerEntities) {
        _session.sendTcp(playerId, msg);
    }

    LOG_DEBUG("Sent DEATH for enemy " + std::to_string(enemy) + " to all players");
}

void GameHandler::sendDestroyedPlayerToAllPlayers(Entity player) {
    auto &factory = MessageFactory::getInstance();
    MessageData payload = factory.encodeMessageDeath(EntityType::PLAYER, player);
    PreparedMessage msg = factory.createMessage(OpCode::DEATH, payload);

    for (const auto &[playerId, entity] : playerEntities) {
        _session.sendTcp(playerId, msg);
    }

    LOG_DEBUG("Sent DEATH for player " + std::to_string(player) + " to all players");
}

void GameHandler::sendScoreUpdateToAllPlayers() {
    auto &factory = MessageFactory::getInstance();
    MessageData payload =
        factory.encodeMessageScoreUpdate(static_cast<uint32_t>(score), _participantCount);
    PreparedMessage msg = factory.createMessage(OpCode::SCORE_UPDATE, payload);

    for (const auto &[playerId, entity] : playerEntities) {
        _session.sendTcp(playerId, msg);
    }

    LOG_DEBUG("Sent SCORE_UPDATE (" + std::to_string(score) + " / " +
              std::to_string(_participantCount) + ") to all players");
}

void GameHandler::onPlayerConnect(const Player &player) {
    LOG_DEBUG("onPlayerConnect START for player " + std::to_string(player.id));

    // Increment participant count (total players who ever joined)
    _participantCount++;

    if (_waitingForUpgrades) {
        LOG_INFO("Player " + std::to_string(player.id) +
                 " connected during upgrade phase. Added to pending list.");
        _pendingPlayers.push_back(player);
        return;
    }

    LOG_DEBUG("onPlayerConnect: getting config");
    const auto &pStats = _config.getPlayerConfig().stats;
    LOG_DEBUG("onPlayerConnect: creating entity");
    Entity playerEntity = reg.createEntity();
    LOG_DEBUG("onPlayerConnect: adding Position");
    reg.addComponent<Position>(playerEntity, _config.getPlayerConfig().initial_x,
                               _config.getPlayerConfig().initial_y);
    LOG_DEBUG("onPlayerConnect: adding Velocity");
    reg.addComponent<Velocity>(playerEntity, 0.f, 0.f);

    // Initialize Stats from Config
    LOG_DEBUG("onPlayerConnect: adding Stats");
    reg.addComponent<Stats>(playerEntity, pStats.health, pStats.max_health, pStats.attack_speed,
                            0.f, pStats.attack_damage, 1, pStats.speed);

    LOG_DEBUG("onPlayerConnect: setting weapon type");
    weaponSystem.setWeaponType(reg, playerEntity, WeaponType::DEFAULT);

    // Override with config stats
    if (reg.hasComponent<Weapon>(playerEntity)) {
        auto &w = reg.getComponent<Weapon>(playerEntity);
        w.damage = pStats.attack_damage;
        if (pStats.attack_speed > 0) {
            w.fireRate = 5.0f / (float)pStats.attack_speed;
        }
    }

    auto &factory = MessageFactory::getInstance();

    // Find the first available skin index (0-3) that isn't already in use
    uint8_t skinIndex = 0;
    for (uint8_t i = 0; i < 4; ++i) {
        bool inUse = false;
        for (const auto &[pid, idx] : playerSkinIndices) {
            if (idx == i) {
                inUse = true;
                break;
            }
        }
        if (!inUse) {
            skinIndex = i;
            break;
        }
    }
    playerSkinIndices[player.id] = skinIndex;

    // Send all existing players' info to the new player
    for (const auto &[existingPlayerId, existingEntity] : playerEntities) {
        Position &pos = reg.getComponent<Position>(existingEntity);
        uint8_t existingSkinIndex =
            playerSkinIndices.count(existingPlayerId) ? playerSkinIndices[existingPlayerId] : 0;
        MessageData payload = factory.encodePlayerInfo(existingPlayerId, existingEntity, pos.x,
                                                       pos.y, existingSkinIndex);
        PreparedMessage msg = factory.createMessage(OpCode::PLAYER, payload);
        _session.sendTcp(player.id, msg);
        LOG_DEBUG("Sent existing player " + std::to_string(existingPlayerId) +
                  " info to new player " + std::to_string(player.id));
    }

    for (auto enemyEntity : reg.viewEntitiesWith<Enemy, Position>()) {
        auto &pos = reg.getComponent<Position>(enemyEntity);
        auto &enemy = reg.getComponent<Enemy>(enemyEntity);

        MessageData payload = factory.encodeMessageEnemy(enemyEntity, pos.x, pos.y, enemy.type);
        PreparedMessage msg = factory.createMessage(OpCode::ENEMY, payload);
        _session.sendUdp(player.id, msg);
        LOG_DEBUG("Sent existing enemy " + std::to_string(enemyEntity) + " info to new player " +
                  std::to_string(player.id));
    }

    // Send current score to new player
    if (score > 0) {
        MessageData scorePayload =
            factory.encodeMessageScoreUpdate(static_cast<uint32_t>(score), _participantCount);
        PreparedMessage scoreMsg = factory.createMessage(OpCode::SCORE_UPDATE, scorePayload);
        _session.sendTcp(player.id, scoreMsg);
        LOG_DEBUG("Sent current score " + std::to_string(score) + " / " +
                  std::to_string(_participantCount) + " to new player " +
                  std::to_string(player.id));
    }

    for (auto projEntity : reg.viewEntitiesWith<Projectile, Position>()) {
        auto &pos = reg.getComponent<Position>(projEntity);
        auto &proj = reg.getComponent<Projectile>(projEntity);

        float vx = 0.f, vy = 0.f;
        if (reg.hasComponent<Velocity>(projEntity)) {
            auto &vel = reg.getComponent<Velocity>(projEntity);
            vx = vel.vx;
            vy = vel.vy;
        }

        Entity parentEntity = 0;
        MessageData payload = factory.encodeMessageProjectile(
            projEntity, parentEntity, proj.ownerType, pos.x, pos.y, vx, vy, proj.scale);
        PreparedMessage msg = factory.createMessage(OpCode::SHOOT, payload);
        _session.sendTcp(player.id, msg);
        LOG_DEBUG("Sent existing projectile " + std::to_string(projEntity) +
                  " info to new player " + std::to_string(player.id));
    }

    // Add new player to the map
    playerEntities[player.id] = playerEntity;

    if (playerEntities.size() >= 1 && !_gameStarted) {
        _gameStarted = true;
        LOG_INFO("Minimum players reached. Game started!");
        enemySystem.startSpawning();
    }

    Position &newPlayerPos = reg.getComponent<Position>(playerEntity);
    MessageData payload = factory.encodePlayerInfo(player.id, playerEntity, newPlayerPos.x,
                                                   newPlayerPos.y, skinIndex);
    PreparedMessage msg = factory.createMessage(OpCode::PLAYER, payload);
    for (const auto &[existingPlayerId, existingEntity] : playerEntities) {
        _session.sendTcp(existingPlayerId, msg);
        LOG_DEBUG("Sent new player " + std::to_string(player.id) + " info to player " +
                  std::to_string(existingPlayerId));
    }

    // Sync Stats and Weapon to all players for the new player
    if (reg.hasComponent<Stats>(playerEntity)) {
        auto &s = reg.getComponent<Stats>(playerEntity);
        MessageData statsPayload =
            factory.encodeMessageUpdateStats(playerEntity, s.hp, s.maxHp, s.movement_speed);
        PreparedMessage statsMsg = factory.createMessage(OpCode::UPDATE_STATS, statsPayload);
        for (const auto &[pid, _] : playerEntities) {
            _session.sendTcp(pid, statsMsg);
        }
    }
    if (reg.hasComponent<Weapon>(playerEntity)) {
        auto &w = reg.getComponent<Weapon>(playerEntity);
        MessageData weaponPayload =
            factory.encodeMessageUpdateWeapon(playerEntity, w.damage, w.nbOfBullets, w.fireRate);
        PreparedMessage weaponMsg = factory.createMessage(OpCode::UPDATE_WEAPON, weaponPayload);
        for (const auto &[pid, _] : playerEntities) {
            _session.sendTcp(pid, weaponMsg);
        }
    }

    LOG_INFO("Player " + std::to_string(player.id) + " entity created and synced with " +
             std::to_string(playerEntities.size() - 1) + " other players");
}

void GameHandler::onPlayerDisconnect(uint32_t playerId) {
    auto it = playerEntities.find(playerId);
    if (it != playerEntities.end()) {
        Entity playerEntity = it->second;

        // Send DEATH message for the disconnected player to all remaining players
        auto &factory = MessageFactory::getInstance();
        {
            MessageData payload = factory.encodeMessageDeath(EntityType::PLAYER, playerEntity);
            PreparedMessage msg = factory.createMessage(OpCode::DEATH, payload);
            for (const auto &[pid, entity] : playerEntities) {
                if (pid != playerId) {
                    _session.sendTcp(pid, msg);
                }
            }
        }

        std::vector<Entity> companionsToRemove;
        for (auto e : reg.viewEntitiesWith<Parent>()) {
            if (reg.getComponent<Parent>(e).entity == playerEntity) {
                companionsToRemove.push_back(e);
            }
        }

        for (auto companion : companionsToRemove) {
            MessageData payload = factory.encodeMessageDeath(EntityType::COMPANION, companion);
            PreparedMessage msg = factory.createMessage(OpCode::DEATH, payload);
            for (const auto &[pid, entity] : playerEntities) {
                if (pid != playerId) {
                    _session.sendTcp(pid, msg);
                }
            }
            reg.destroyEntity(companion);
        }

        reg.destroyEntity(playerEntity);
        playerEntities.erase(it);
        playerSkinIndices.erase(playerId); // Clean up skin index
        LOG_INFO("Player " + std::to_string(playerId) + " entity destroyed");

        if (_waitingForUpgrades) {
            _playersSelectedUpgrade.erase(playerId);
            if (playerEntities.empty() || _playersSelectedUpgrade.size() >= playerEntities.size()) {
                _waitingForUpgrades = false;
                _offeredUpgrades.clear();
                enemySystem.setWavePaused(false);
                LOG_INFO("All remaining players selected upgrades. Resuming wave.");

                // Process pending players
                for (const auto &pending : _pendingPlayers) {
                    onPlayerConnect(pending);
                }
                _pendingPlayers.clear();
            }
        }
    }
}

void GameHandler::onPlayerMove(const MoveData &moveData) {
    auto it = playerEntities.find(moveData.playerId);
    if (it == playerEntities.end()) {
        LOG_WARN("Move received for unknown player: " + std::to_string(moveData.playerId));
        return;
    }

    Entity entity = it->second;
    if (reg.hasComponent<Velocity>(entity)) {
        auto &velocity = reg.getComponent<Velocity>(entity);
        velocity.vx = moveData.vx;
        velocity.vy = moveData.vy;
        LOG_DEBUG("Player " + std::to_string(moveData.playerId) + " velocity updated to (" +
                  std::to_string(moveData.vx) + ", " + std::to_string(moveData.vy) + ")");

        auto &position = reg.getComponent<Position>(entity);
        LOG_DEBUG("Player " + std::to_string(moveData.playerId) + " position is (" +
                  std::to_string(position.x) + ", " + std::to_string(position.y) + ")");
    }
}

void GameHandler::onPlayerShoot(const ShootData &shootData) {
    auto it = playerEntities.find(shootData.playerId);
    if (it == playerEntities.end()) {
        LOG_WARN("Shoot received for unknown player: " + std::to_string(shootData.playerId));
        return;
    }

    Entity entity = it->second;
    if (!reg.hasComponent<Stats>(entity)) {
        LOG_WARN("Shoot received for player without Stats component: " +
                 std::to_string(shootData.playerId));
        return;
    }

    if (!reg.hasComponent<Weapon>(entity)) {
        LOG_WARN("Shoot received for player without Weapon component: " +
                 std::to_string(shootData.playerId));
        return;
    }

    weaponSystem.fireWeapon(reg, entity);

    // Fire companion weapons
    for (auto companionEntity : reg.viewEntitiesWith<Parent, Weapon>()) {
        if (!reg.hasComponent<Parent>(companionEntity))
            continue;
        auto &parent = reg.getComponent<Parent>(companionEntity);
        if (parent.entity == entity) {
            weaponSystem.fireWeapon(reg, companionEntity);
        }
    }
}

void GameHandler::initNewEnemyEntities(const std::vector<Entity> &newEnemyEntities) {
    for (const auto &enemyEntity : newEnemyEntities) {
        if (!reg.hasComponent<Position>(enemyEntity))
            continue;

        Position &pos = reg.getComponent<Position>(enemyEntity);
        std::string type = "basic_enemy";
        if (reg.hasComponent<Enemy>(enemyEntity)) {
            type = reg.getComponent<Enemy>(enemyEntity).type;
        }

        MessageData payload =
            MessageFactory::getInstance().encodeMessageEnemy(enemyEntity, pos.x, pos.y, type);
        PreparedMessage msg = MessageFactory::getInstance().createMessage(OpCode::ENEMY, payload);

        for (const auto &[playerId, playerEntity] : playerEntities) {
            _session.sendUdp(playerId, msg);
            LOG_DEBUG("Sent new enemy entity " + std::to_string(enemyEntity) + " (" + type +
                      ") to player " + std::to_string(playerId));
        }
    }
}

void GameHandler::updateEnemyPosition(const std::vector<Entity> &allEnemyEntities) {
    auto &factory = MessageFactory::getInstance();

    for (const auto &enemyEntity : allEnemyEntities) {
        if (!reg.hasComponent<Position>(enemyEntity))
            continue;

        Position &pos = reg.getComponent<Position>(enemyEntity);
        auto vector = reg.hasComponent<Velocity>(enemyEntity)
                          ? reg.getComponent<Velocity>(enemyEntity)
                          : Velocity{0.f, 0.f};
        if (vector.vx == 0.f && vector.vy == 0.f)
            continue; // enemy is not moving no update
        MessageData payload =
            factory.encodeMessageMove(EntityType::ENEMY, enemyEntity, pos.x, pos.y);
        PreparedMessage msg = factory.createMessage(OpCode::MOVE_SYNC, payload);

        for (const auto &[playerId, playerEntity] : playerEntities) {
            _session.sendUdp(playerId, msg);
        }
    }
}

void GameHandler::onPlayerLinked(uint32_t playerId) {
    auto &factory = MessageFactory::getInstance();

    for (Entity enemyEntity : enemySystem.getEnemyEntities()) {
        if (!reg.hasComponent<Position>(enemyEntity))
            continue;

        Position &pos = reg.getComponent<Position>(enemyEntity);
        std::string type = "basic_enemy";
        if (reg.hasComponent<Enemy>(enemyEntity)) {
            type = reg.getComponent<Enemy>(enemyEntity).type;
        }

        MessageData payload = factory.encodeMessageEnemy(enemyEntity, pos.x, pos.y, type);
        PreparedMessage msg = factory.createMessage(OpCode::ENEMY, payload);
        _session.sendUdp(playerId, msg);
        LOG_DEBUG("Sent existing enemy " + std::to_string(enemyEntity) +
                  " to newly linked player " + std::to_string(playerId));
    }

    // Send existing companions
    for (Entity companion : reg.viewEntitiesWith<Parent, Position>()) {
        if (!reg.hasComponent<Parent>(companion))
            continue;
        Position &pos = reg.getComponent<Position>(companion);

        uint8_t type = 0;
        if (reg.hasComponent<Weapon>(companion)) {
            Weapon &w = reg.getComponent<Weapon>(companion);
            if (w.damage > 20)
                type = 1; // Missile
        }

        MessageData payload = factory.encodeMessageCompanion(companion, pos.x, pos.y, type);
        PreparedMessage msg = factory.createMessage(OpCode::COMPANION, payload);
        _session.sendUdp(playerId, msg);
        LOG_DEBUG("Sent existing companion " + std::to_string(companion) +
                  " to newly linked player " + std::to_string(playerId));
    }
}

void GameHandler::checkPlayerCollisions() {
    std::vector<Entity> projectilesToRemove;
    std::vector<Entity> playersToKill;

    // Config values
    float pW = _config.getPlayerConfig().hitbox.width;
    float pH = _config.getPlayerConfig().hitbox.height;
    float offX = _config.getPlayerConfig().hitbox.offset_x;
    float offY = _config.getPlayerConfig().hitbox.offset_y;

    static int logCounter = 0;
    if (logCounter++ % 120 == 0) {
        LOG_INFO("COLLISION CHECK: W=" + std::to_string(pW) + " H=" + std::to_string(pH) +
                 " OffX=" + std::to_string(offX) + " OffY=" + std::to_string(offY));
    }

    // View all projectiles
    for (auto projectileEntity : reg.viewEntitiesWith<Projectile, Position>()) {
        if (!reg.hasComponent<Projectile>(projectileEntity))
            continue; // Safety check
        auto &projectile = reg.getComponent<Projectile>(projectileEntity);

        // Only check enemy projectiles against players
        if (projectile.ownerType == "player")
            continue;

        Position &projPos = reg.getComponent<Position>(projectileEntity);

        // Check against all players
        for (const auto &[playerId, playerEntity] : playerEntities) {
            // Check validity
            if (!reg.hasComponent<Position>(playerEntity) || !reg.hasComponent<Stats>(playerEntity))
                continue;

            Position &playerPos = reg.getComponent<Position>(playerEntity);
            Stats &playerStats = reg.getComponent<Stats>(playerEntity);

            // Skip dead players if we want to avoid beating a dead horse
            if (playerStats.hp <= 0)
                continue;

            // Simple AABB Collision

            float targetX = playerPos.x + offX;
            float targetY = playerPos.y + offY;

            float projW = 16.0f * projectile.scale;
            float projH = 16.0f * projectile.scale;
            float projLeft = projPos.x;
            float projTop = projPos.y;

            // Debug logging every 60 frames for first player
            static int debugLogCounter = 0;
            if (debugLogCounter++ % 60 == 0 && playerId == playerEntities.begin()->first) {
                LOG_DEBUG("COLLISION DEBUG: PlayerPos(" + std::to_string(playerPos.x) + ", " +
                          std::to_string(playerPos.y) + ") -> Hitbox(" + std::to_string(targetX) +
                          ", " + std::to_string(targetY) + ", " + std::to_string(pW) + ", " +
                          std::to_string(pH) + ")");
            }

            bool collision = (projLeft < targetX + pW && projLeft + projW > targetX &&
                              projTop < targetY + pH && projTop + projH > targetY);

            if (collision) {
                LOG_INFO("HIT! Projectile(" + std::to_string(projLeft) + ", " +
                         std::to_string(projTop) + ", " + std::to_string(projW) + ", " +
                         std::to_string(projH) + ") vs PlayerHitbox(" + std::to_string(targetX) +
                         ", " + std::to_string(targetY) + ", " + std::to_string(pW) + ", " +
                         std::to_string(pH) + ")");
                // Hit
                playerStats.hp -= projectile.damage;
                projectilesToRemove.push_back(projectileEntity);

                LOG_DEBUG("Player " + std::to_string(playerId) + " hit by projectile " +
                          std::to_string(projectileEntity) +
                          ", hp: " + std::to_string(playerStats.hp));

                // Send Stats Update
                MessageFactory &factory = MessageFactory::getInstance();
                MessageData statsPayload = factory.encodeMessageUpdateStats(
                    playerEntity, playerStats.hp, playerStats.maxHp, playerStats.movement_speed);
                PreparedMessage statsMsg =
                    factory.createMessage(OpCode::UPDATE_STATS, statsPayload);
                for (const auto &[pid, _] : playerEntities) {
                    _session.sendTcp(pid, statsMsg);
                }

                if (playerStats.hp <= 0) {
                    playersToKill.push_back(playerEntity);
                }

                // One projectile hits one player
                break;
            }
        }
    }

    // Process removals
    // Remove duplicates from projectilesToRemove if any
    for (auto proj : projectilesToRemove) {
        if (reg.hasComponent<Projectile>(proj)) { // Check if still exists
            sendDestroyedProjectileToAllPlayers(proj);
            reg.destroyEntity(proj);
        }
    }

    // Also check for any players killed by other means (e.g., asteroid collision)
    for (const auto &[playerId, playerEntity] : playerEntities) {
        if (!reg.hasComponent<Stats>(playerEntity))
            continue;
        Stats &stats = reg.getComponent<Stats>(playerEntity);
        if (stats.hp <= 0) {
            // Check if not already in playersToKill
            bool alreadyMarked = false;
            for (auto e : playersToKill) {
                if (e == playerEntity) {
                    alreadyMarked = true;
                    break;
                }
            }
            if (!alreadyMarked) {
                playersToKill.push_back(playerEntity);
            }
        }
    }

    // Notify deaths and remove dead players
    std::vector<uint32_t> playerIdsToRemove;
    for (auto player : playersToKill) {
        // Find the playerId for this entity
        uint32_t deadPlayerId = 0;
        for (const auto &pair : playerEntities) {
            if (pair.second == player) {
                deadPlayerId = pair.first;
                break;
            }
        }

        if (deadPlayerId != 0) {
            playerIdsToRemove.push_back(deadPlayerId);
        }

        sendDestroyedPlayerToAllPlayers(player);
        std::vector<Entity> companionsToRemove;
        for (auto e : reg.viewEntitiesWith<Parent>()) {
            if (reg.getComponent<Parent>(e).entity == player) {
                companionsToRemove.push_back(e);
            }
        }

        auto &factory = MessageFactory::getInstance();
        for (auto companion : companionsToRemove) {
            // Send death message for companion
            MessageData payload = factory.encodeMessageDeath(EntityType::COMPANION, companion);
            PreparedMessage msg = factory.createMessage(OpCode::DEATH, payload);
            for (const auto &[pid, entity] : playerEntities) {
                if (entity != 0) {
                    _session.sendUdp(pid, msg);
                }
            }
            reg.destroyEntity(companion);
        }
        reg.destroyEntity(player);
    }

    // Remove dead players from playerEntities map
    for (uint32_t pid : playerIdsToRemove) {
        playerEntities.erase(pid);
        LOG_INFO("Player " + std::to_string(pid) + " died and removed from game");

        // Notify Room to remove player from its list
        if (_onPlayerDeath) {
            _onPlayerDeath(pid);
        }
    }
}

void GameHandler::onUpgradeSelect(uint32_t playerId, uint8_t index) {
    if (!_waitingForUpgrades)
        return;
    if (index >= _offeredUpgrades.size())
        return;

    if (_playersSelectedUpgrade.find(playerId) != _playersSelectedUpgrade.end()) {
        return;
    }

    // Apply upgrade
    const auto &upgrade = _offeredUpgrades[index];
    LOG_INFO("Player " + std::to_string(playerId) + " selected upgrade: " + upgrade.name);

    _playersSelectedUpgrade.insert(playerId);

    MessageFactory &factory = MessageFactory::getInstance();

    auto it = playerEntities.find(playerId);
    if (it != playerEntities.end()) {
        Entity entity = it->second;

        if (reg.hasComponent<Stats>(entity)) {
            Stats &stats = reg.getComponent<Stats>(entity);

            bool weaponUpdated = false;
            Weapon *weapon = nullptr;
            if (reg.hasComponent<Weapon>(entity)) {
                weapon = &reg.getComponent<Weapon>(entity);
            }

            for (const auto &effect : upgrade.effects) {
                if (effect.target == "max_health") {
                    stats.maxHp += (int)effect.value;
                    stats.hp += (int)effect.value;
                } else if (effect.target == "movement_speed") {
                    stats.movement_speed =
                        (int)(stats.movement_speed * (1.0f + effect.value / 100.0f));
                } else if (effect.target == "damage_multiplier") {
                    stats.attack_damage = (int)(stats.attack_damage * (1.0f + effect.value));
                    if (weapon) {
                        weapon->damage = (int)(weapon->damage * (1.0f + effect.value));
                        weaponUpdated = true;
                    }
                } else if (effect.target == "fire_rate") {
                    stats.attack_speed = (int)(stats.attack_speed * (1.0f + effect.value));
                    if (weapon) {
                        // Increase fire rate means decrease delay
                        if (effect.value > -1.0f) // Prevent division by zero or negative
                            weapon->fireRate /= (1.0f + effect.value);
                        weaponUpdated = true;
                    }
                } else if (effect.target == "projectile_scale") {
                    if (weapon) {
                        weapon->projectileScale += effect.value;
                        weaponUpdated = true;
                    }
                } else if (effect.target == "add_weapon_shotgun") {
                    LOG_INFO("Spawning SHOTGUN companion for player " + std::to_string(playerId));
                    spawnCompanion(entity, WeaponType::SHOTGUN);
                } else if (effect.target == "add_weapon_missile") {
                    LOG_INFO("Spawning MISSILE companion for player " + std::to_string(playerId));
                    spawnCompanion(entity, WeaponType::MISSILE);
                } else if (effect.target == "extra_projectiles") {
                    if (weapon) {
                        weapon->nbOfBullets += (int)effect.value;
                        weaponUpdated = true;
                    }
                } else if (effect.target == "add_diagonal") {
                    if (weapon) {
                        if (weapon->nbOfBullets < 100) {
                            weapon->nbOfBullets += 100;
                            weaponUpdated = true;
                        }
                    }
                } else if (effect.target == "current_health_percent") {
                    stats.hp += (int)(stats.maxHp * (effect.value / 100.0f));
                    if (stats.hp > stats.maxHp)
                        stats.hp = stats.maxHp;
                }
            }

            if (weaponUpdated && weapon) {
                MessageData payload = factory.encodeMessageUpdateWeapon(
                    entity, weapon->damage, weapon->nbOfBullets, weapon->fireRate);
                PreparedMessage msg = factory.createMessage(OpCode::UPDATE_WEAPON, payload);

                // Broadcast to all
                for (const auto &[targetPid, _] : playerEntities) {
                    _session.sendTcp(targetPid, msg);
                }
            }

            // Send Stats Update
            MessageData statsPayload = factory.encodeMessageUpdateStats(
                entity, stats.hp, stats.maxHp, stats.movement_speed);
            PreparedMessage statsMsg = factory.createMessage(OpCode::UPDATE_STATS, statsPayload);
            for (const auto &[targetPid, _] : playerEntities) {
                _session.sendTcp(targetPid, statsMsg);
            }
        }
    }

    // Check for resume
    if (_playersSelectedUpgrade.size() >= playerEntities.size()) {
        _waitingForUpgrades = false;
        _offeredUpgrades.clear();
        enemySystem.setWavePaused(false);
        LOG_INFO("All players selected upgrades. Resuming wave.");

        // Process pending players
        for (const auto &pending : _pendingPlayers) {
            onPlayerConnect(pending);
        }
        _pendingPlayers.clear();
    } else {
        LOG_INFO("Waiting for " +
                 std::to_string(playerEntities.size() - _playersSelectedUpgrade.size()) +
                 " more players.");
    }
}

void GameHandler::spawnCompanion(Entity parent, WeaponType weaponType) {
    LOG_INFO("Entering spawnCompanion for parent " + std::to_string(parent));
    if (!reg.hasComponent<Position>(parent)) {
        LOG_WARN("Parent has no Position component!");
        return;
    }
    auto &pos = reg.getComponent<Position>(parent);

    // Count existing companions
    int companionCount = 0;
    for (auto ent : reg.viewEntitiesWith<Parent>()) {
        if (!reg.hasComponent<Parent>(ent))
            continue;
        if (reg.getComponent<Parent>(ent).entity == parent)
            companionCount++;
    }

    float offX = -30.0f;
    float offY = (companionCount % 2 == 0) ? -50.0f : 50.0f;
    if (companionCount >= 2)
        offY *= (1.0f + (companionCount / 2) * 0.5f);

    float startX = pos.x + offX;
    float startY = pos.y + offY;

    Entity drone = reg.createEntity();
    reg.addComponent<Position>(drone, startX, startY);
    reg.addComponent<Parent>(drone, parent, offX, offY);
    reg.addComponent<Velocity>(drone, 0.f, 0.f);

    weaponSystem.setWeaponType(reg, drone, weaponType);

    // Configure Drone specific weapon offsets
    if (reg.hasComponent<Weapon>(drone)) {
        auto &w = reg.getComponent<Weapon>(drone);
        w.offsetX = 10.0f;
        w.offsetY = 12.0f;
    }

    // Notify clients of new companion
    uint8_t type = (weaponType == WeaponType::SHOTGUN) ? 0 : 1;

    MessageData payload =
        MessageFactory::getInstance().encodeMessageCompanion(drone, startX, startY, type);
    PreparedMessage msg = MessageFactory::getInstance().createMessage(OpCode::COMPANION, payload);

    for (const auto &[pid, _] : playerEntities) {
        _session.sendTcp(pid, msg);
    }

    LOG_INFO("Spawned companion " + std::to_string(drone) + " for player " +
             std::to_string(parent));
}
