#include "GameHandler.hpp"
#include "../Logs/Logger.hpp"
#include "../../common/Data/EntityType.hpp"

#include <chrono>
#include <thread>

GameHandler::GameHandler(SessionManager& session, std::atomic<bool>& running, const std::string& configPath)
    : _running(running)
    , _session(session)
    , _messageHandler(session, running)
{
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
    
    // Print verify loaded values
    auto& hb = _config.getPlayerConfig().hitbox;
    LOG_INFO("Loaded Player Hitbox: W=" + std::to_string(hb.width) + " H=" + std::to_string(hb.height) + 
             " OffX=" + std::to_string(hb.offset_x) + " OffY=" + std::to_string(hb.offset_y));


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
    auto& projConfig = _config.getProjectilesConfig().enemy;
    enemySystem.setProjectileOffsets(projConfig.offset_x, projConfig.offset_y);
    LOG_INFO("Enemy projectile offsets: X=" + std::to_string(projConfig.offset_x) + " Y=" + std::to_string(projConfig.offset_y));

    // Set up message handler callbacks
    _messageHandler.setOnPlayerConnect([this](const Player& player) {
        onPlayerConnect(player);
    });
    
    _messageHandler.setOnPlayerMove([this](const MoveData& moveData) {
        onPlayerMove(moveData);
    });

    _messageHandler.setOnPlayerShoot([this](const ShootData& shootData) {
        onPlayerShoot(shootData);
    });
    
    _messageHandler.setOnPlayerLink([this](uint32_t playerId) {
        onPlayerLinked(playerId);
    });
    
    // Set up session manager callback for disconnections
    _session.setOnPlayerDisconnect([this](const Player& player) {
        onPlayerDisconnect(player);
    });
    
    ScoreEntity = reg.createEntity();
    reg.addComponent<Position>(ScoreEntity, 0.f, 0.f);
    reg.addComponent<Label>(ScoreEntity, std::string("Score: 0"), std::string("font/josefin-sans/JosefinSans-Regular.ttf"), std::string("default_font"), Color(255, 255, 255), 0, true);

    weaponSystem.setProjectileNotifier([this](Entity parent, Entity projectile) {
        sendNewProjectilesToAllPlayers(parent, projectile);
    });
    
    // Configure projectile offsets and speed from config
    weaponSystem.setProjectileConfig(
        _config.getProjectilesConfig().player.offset_x,
        _config.getProjectilesConfig().player.offset_y,
        _config.getProjectilesConfig().player.speed
    );
}

void GameHandler::run()
{
    LOG_INFO("Game loop started");
    
    auto lastTime = std::chrono::steady_clock::now();
    constexpr float targetFrameTime = 1.0f / 30.0f; // 30 ticks per second

    while (_running) {
        auto currentTime = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Process all pending messages
        processMessages();
        
        // Update game state
        updateGame(deltaTime);

        // Send game packet to players
        sendUpdatedPositionToAllPlayers();

        // Frame rate limiting
        auto frameEnd = std::chrono::steady_clock::now();
        float frameTime = std::chrono::duration<float>(frameEnd - currentTime).count();
        if (frameTime < targetFrameTime) {
            std::this_thread::sleep_for(
                std::chrono::duration<float>(targetFrameTime - frameTime)
            );
        }
    }
    
    LOG_INFO("Game loop stopped");
}

void GameHandler::sendUpdatedPositionToAllPlayers()
{
    for (const auto& [playerId, entity] : playerEntities) {
        sendUpdatedPositionToPlayer(playerId);
    }
    for (const auto& [playerId, entity] : playerEntities) {
        wasMoving[entity] = reg.hasComponent<Velocity>(entity) &&
                             (reg.getComponent<Velocity>(entity).vx != 0.f || reg.getComponent<Velocity>(entity).vy != 0.f);
    }
}

void GameHandler::sendNewProjectilesToAllPlayers(Entity parentEntity, Entity projectileEntity)
{
    std::string ownerType = "player";
    if (reg.hasComponent<Projectile>(projectileEntity)) {
        ownerType = reg.getComponent<Projectile>(projectileEntity).ownerType;
    }
    
    MessageData payload = MessageFactory::getInstance().encodeMessageProjectile(projectileEntity, parentEntity, ownerType);
    PreparedMessage msg = MessageFactory::getInstance().createMessage(OpCode::SHOOT, payload);
    
    for (const auto& [playerId, entity] : playerEntities) {
        _session.sendTcp(playerId, msg);
    }
    
    LOG_DEBUG("Sent projectile " + std::to_string(projectileEntity) + " from parent " + std::to_string(parentEntity) + " to all players");
}

/// @brief send cur position of all entity containing a vector to a player
/// @param playerId the id of the player to send the position to
void GameHandler::sendUpdatedPositionToPlayer(uint32_t playerId)
{
    auto& factory = MessageFactory::getInstance();
    
    // Send player positions
    for (const auto& [pid, entity] : playerEntities) {
        Position& pos = reg.getComponent<Position>(entity);
        if (!reg.hasComponent<Velocity>(entity))
            continue;
        
        bool isMoving = reg.getComponent<Velocity>(entity).vx != 0.f || reg.getComponent<Velocity>(entity).vy != 0.f;
        bool entityWasMoving = wasMoving[entity];
        
        if (isMoving || entityWasMoving) {
            MessageData payload = factory.encodeMessageMove(EntityType::PLAYER, entity, pos.x, pos.y);
            PreparedMessage msg = factory.createMessage(OpCode::MOVE_SYNC, payload);  // <-- MOVE_SYNC
            _session.sendUdp(playerId, msg);
        }
    }
}

void GameHandler::processMessages()
{
    // Process up to a maximum number of messages per frame to avoid starvation
    constexpr int maxMessagesPerFrame = 100;
    int processed = 0;
    
    while (processed < maxMessagesPerFrame && _messageHandler.processSingleCycle()) {
        processed++;
    }
}

void GameHandler::updateGame(float deltaTime)
{
    enemySystem.update(reg, deltaTime);
    
    for (const auto& [enemy, projectile] : enemySystem.getNewProjectileEntitiesWithParent()) {
        sendNewProjectilesToAllPlayers(enemy, projectile);
    }
    for (const auto& projectile : enemySystem.getProjectileColliding()) {
        sendDestroyedProjectileToAllPlayers(projectile);
        reg.destroyEntity(projectile);
    }
    for (const auto& enemy : enemySystem.getDeadEnemyEntities()) {
        sendDestroyedEnemyToAllPlayers(enemy);
    }
    
    initNewEnemyEntities(enemySystem.getNewEnemyEntities());
    updateEnemyPosition(enemySystem.getEnemyEntities());
    movement.update(reg, deltaTime);
    statsys.update(reg, deltaTime);
    weaponSystem.update(reg, deltaTime);
    
    checkPlayerCollisions();
}

void GameHandler::sendDestroyedProjectileToAllPlayers(Entity projectile)
{
    auto& factory = MessageFactory::getInstance();
    MessageData payload = factory.encodeMessageDeath(EntityType::PROJECTILE, projectile);
    PreparedMessage msg = factory.createMessage(OpCode::DEATH, payload);
    
    for (const auto& [playerId, entity] : playerEntities) {
        _session.sendUdp(playerId, msg);
    }
    
    LOG_DEBUG("Sent DEATH for projectile " + std::to_string(projectile) + " to all players");
}

void GameHandler::sendDestroyedEnemyToAllPlayers(Entity enemy)
{
    auto& factory = MessageFactory::getInstance();
    MessageData payload = factory.encodeMessageDeath(EntityType::ENEMY, enemy);
    PreparedMessage msg = factory.createMessage(OpCode::DEATH, payload);
    
    for (const auto& [playerId, entity] : playerEntities) {
        _session.sendUdp(playerId, msg);
    }
    
    LOG_DEBUG("Sent DEATH for enemy " + std::to_string(enemy) + " to all players");
}

void GameHandler::sendDestroyedPlayerToAllPlayers(Entity player)
{
    auto& factory = MessageFactory::getInstance();
    MessageData payload = factory.encodeMessageDeath(EntityType::PLAYER, player);
    PreparedMessage msg = factory.createMessage(OpCode::DEATH, payload);
    
    for (const auto& [playerId, entity] : playerEntities) {
        _session.sendUdp(playerId, msg);
    }
    
    LOG_DEBUG("Sent DEATH for player " + std::to_string(player) + " to all players");
}

void GameHandler::onPlayerConnect(const Player& player)
{
    Entity playerEntity = reg.createEntity();
    reg.addComponent<Position>(playerEntity, _config.getPlayerConfig().initial_x, _config.getPlayerConfig().initial_y);
    reg.addComponent<Velocity>(playerEntity, 0.f, 0.f);
    reg.addComponent<Stats>(playerEntity, 100, 100, 1, 0.f, 10, 1, 200);
    reg.addComponent<Weapon>(playerEntity, 10, 1, 0.5f, false);
    
    auto& factory = MessageFactory::getInstance();
    
    // Send all existing players' info to the new player
    for (const auto& [existingPlayerId, existingEntity] : playerEntities) {
        Position& pos = reg.getComponent<Position>(existingEntity);
        MessageData payload = factory.encodePlayerInfo(existingPlayerId, existingEntity, pos.x, pos.y);
        PreparedMessage msg = factory.createMessage(OpCode::PLAYER, payload);
        _session.sendTcp(player.id, msg);
        LOG_DEBUG("Sent existing player " + std::to_string(existingPlayerId) + " info to new player " + std::to_string(player.id));
    }
    
    // Add new player to the map
    playerEntities[player.id] = playerEntity;

    if (playerEntities.size() >= 1 && !_gameStarted) {
        _gameStarted = true;
        LOG_INFO("Minimum players reached. Game started!");
        enemySystem.startSpawning();
    }
    
    // Send the new player's info to all players (including themselves)
    Position& newPlayerPos = reg.getComponent<Position>(playerEntity);
    MessageData payload = factory.encodePlayerInfo(player.id, playerEntity, newPlayerPos.x, newPlayerPos.y);
    PreparedMessage msg = factory.createMessage(OpCode::PLAYER, payload);
    for (const auto& [existingPlayerId, existingEntity] : playerEntities) {
        _session.sendTcp(existingPlayerId, msg);
        LOG_DEBUG("Sent new player " + std::to_string(player.id) + " info to player " + std::to_string(existingPlayerId));
    }
    
    LOG_INFO("Player " + std::to_string(player.id) + " entity created and synced with " + std::to_string(playerEntities.size() - 1) + " other players");
}

void GameHandler::onPlayerDisconnect(const Player& player)
{
    auto it = playerEntities.find(player.id);
    if (it != playerEntities.end()) {
        reg.destroyEntity(it->second);
        playerEntities.erase(it);
        LOG_INFO("Player " + std::to_string(player.id) + " entity destroyed");
    }
}

void GameHandler::onPlayerMove(const MoveData& moveData)
{
    auto it = playerEntities.find(moveData.playerId);
    if (it == playerEntities.end()) {
        LOG_WARN("Move received for unknown player: " + std::to_string(moveData.playerId));
        return;
    }
    
    Entity entity = it->second;
    if (reg.hasComponent<Velocity>(entity)) {
        auto& velocity = reg.getComponent<Velocity>(entity);
        velocity.vx = moveData.vx;
        velocity.vy = moveData.vy;
        LOG_DEBUG("Player " + std::to_string(moveData.playerId) + " velocity updated to (" + 
                  std::to_string(moveData.vx) + ", " + std::to_string(moveData.vy) + ")");

        auto & position = reg.getComponent<Position>(entity);
        LOG_DEBUG("Player " + std::to_string(moveData.playerId) + " position is (" + 
                  std::to_string(position.x) + ", " + std::to_string(position.y) + ")");
    }
}

void GameHandler::onPlayerShoot(const ShootData& shootData)
{
    auto it = playerEntities.find(shootData.playerId);
    if (it == playerEntities.end()) {
        LOG_WARN("Shoot received for unknown player: " + std::to_string(shootData.playerId));
        return;
    }
    
    Entity entity = it->second;
    if (!reg.hasComponent<Stats>(entity)) {
        LOG_WARN("Shoot received for player without Stats component: " + std::to_string(shootData.playerId));
        return;
    }

    if (!reg.hasComponent<Weapon>(entity)) {
        LOG_WARN("Shoot received for player without Weapon component: " + std::to_string(shootData.playerId));
        return;
    }

    weaponSystem.fireWeapon(reg, entity);
}

void GameHandler::initNewEnemyEntities(const std::vector<Entity>& newEnemyEntities)
{
    for (const auto& enemyEntity : newEnemyEntities) {
        if (!reg.hasComponent<Position>(enemyEntity)) continue;
        
        Position& pos = reg.getComponent<Position>(enemyEntity);
        MessageData payload = MessageFactory::getInstance().encodeMessageEnemy(enemyEntity, pos.x, pos.y);
        PreparedMessage msg = MessageFactory::getInstance().createMessage(OpCode::ENEMY, payload);
        
        for (const auto& [playerId, playerEntity] : playerEntities) {
            _session.sendUdp(playerId, msg);
            LOG_DEBUG("Sent new enemy entity " + std::to_string(enemyEntity) + " to player " + std::to_string(playerId));
        }
    }
}

void GameHandler::updateEnemyPosition(const std::vector<Entity>& allEnemyEntities)
{
    auto& factory = MessageFactory::getInstance();
    
    for (const auto& enemyEntity : allEnemyEntities) {
        if (!reg.hasComponent<Position>(enemyEntity)) continue;
        
        Position& pos = reg.getComponent<Position>(enemyEntity);
        auto vector = reg.hasComponent<Velocity>(enemyEntity) ? reg.getComponent<Velocity>(enemyEntity) : Velocity{0.f, 0.f};
        if (vector.vx == 0.f && vector.vy == 0.f)
            continue; //enemy is not moving no update
        MessageData payload = factory.encodeMessageMove(EntityType::ENEMY, enemyEntity, pos.x, pos.y);
        PreparedMessage msg = factory.createMessage(OpCode::MOVE_SYNC, payload);
        
        for (const auto& [playerId, playerEntity] : playerEntities) {
            _session.sendUdp(playerId, msg);
        }
    }
}

void GameHandler::onPlayerLinked(uint32_t playerId)
{
    auto& factory = MessageFactory::getInstance();
    
    for (Entity enemyEntity : enemySystem.getEnemyEntities()) {
        if (!reg.hasComponent<Position>(enemyEntity)) continue;
        
        Position& pos = reg.getComponent<Position>(enemyEntity);
        MessageData payload = factory.encodeMessageEnemy(enemyEntity, pos.x, pos.y);
        PreparedMessage msg = factory.createMessage(OpCode::ENEMY, payload);
        _session.sendUdp(playerId, msg);
        LOG_DEBUG("Sent existing enemy " + std::to_string(enemyEntity) + " to newly linked player " + std::to_string(playerId));
    }
}

void GameHandler::checkPlayerCollisions()
{
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
        if (!reg.hasComponent<Projectile>(projectileEntity)) continue; // Safety check
        auto& projectile = reg.getComponent<Projectile>(projectileEntity);
        
        // Only check enemy projectiles against players
        if (projectile.ownerType != "enemy") continue;
        
        Position& projPos = reg.getComponent<Position>(projectileEntity);
        
        // Check against all players
        for (const auto& [playerId, playerEntity] : playerEntities) {
            // Check validity
            if (!reg.hasComponent<Position>(playerEntity) || !reg.hasComponent<Stats>(playerEntity)) continue; 
            
            Position& playerPos = reg.getComponent<Position>(playerEntity);
            Stats& playerStats = reg.getComponent<Stats>(playerEntity);

            // Skip dead players if we want to avoid beating a dead horse
            if (playerStats.hp <= 0) continue;

            // Simple AABB Collision
            // Position (x, y) is the top-left corner of the sprite
            // Hitbox position = Sprite top-left + offset
            
            float targetX = playerPos.x + offX;
            float targetY = playerPos.y + offY;
            
            // Projectile size 16x16 (matching client sprite and debug visualization)
            // Client draws projectile at (projPos.x, projPos.y), so it is Top-Left anchored.
            
            float projW = 16.0f;
            float projH = 16.0f;
            float projLeft = projPos.x;
            float projTop = projPos.y;
            
            // Debug logging every 60 frames for first player
            static int debugLogCounter = 0;
            if (debugLogCounter++ % 60 == 0 && playerId == playerEntities.begin()->first) {
                LOG_DEBUG("COLLISION DEBUG: PlayerPos(" + std::to_string(playerPos.x) + ", " + std::to_string(playerPos.y) + 
                         ") -> Hitbox(" + std::to_string(targetX) + ", " + std::to_string(targetY) + ", " + 
                         std::to_string(pW) + ", " + std::to_string(pH) + ")");
            }
            
            bool collision = (projLeft < targetX + pW &&
                              projLeft + projW > targetX &&
                              projTop < targetY + pH &&
                              projTop + projH > targetY);
            
            if (collision) {
                LOG_INFO("HIT! Projectile(" + std::to_string(projLeft) + ", " + std::to_string(projTop) + 
                         ", " + std::to_string(projW) + ", " + std::to_string(projH) + 
                         ") vs PlayerHitbox(" + std::to_string(targetX) + ", " + std::to_string(targetY) + 
                         ", " + std::to_string(pW) + ", " + std::to_string(pH) + ")");
                // Hit
                playerStats.hp -= projectile.damage;
                projectilesToRemove.push_back(projectileEntity);
                
                LOG_DEBUG("Player " + std::to_string(playerId) + " hit by projectile " + std::to_string(projectileEntity) + ", hp: " + std::to_string(playerStats.hp));

                if (playerStats.hp <= 0) {
                     playersToKill.push_back(playerEntity);
                }
                
                // One projectile hits one player
                break; 
            }
        }
    }

    // Process removals
    // Remove duplicates from projectilesToRemove if any (though break should prevent it)
    for (auto proj : projectilesToRemove) {
        if (reg.hasComponent<Projectile>(proj)) { // Check if still exists
            sendDestroyedProjectileToAllPlayers(proj);
            reg.destroyEntity(proj);
        }
    }
    
    // Notify deaths
    for (auto player : playersToKill) {
        sendDestroyedPlayerToAllPlayers(player);
        // Do NOT destroy player entity here to allow respawn or game over handling
        // Reset position or something? 
        // For now just notify death.
    }
}
