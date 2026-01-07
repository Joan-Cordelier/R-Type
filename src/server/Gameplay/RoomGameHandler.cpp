/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RoomGameHandler implementation
*/

#include "RoomGameHandler.hpp"
#include "../Logs/Logger.hpp"

#include <chrono>
#include <thread>

RoomGameHandler::RoomGameHandler(std::atomic<bool> &running,
                                 SendCallback sendUdp, SendCallback sendTcp,
                                 BroadcastCallback broadcastUdp,
                                 BroadcastCallback broadcastTcp)
    : _running(running), _sendUdp(sendUdp), _sendTcp(sendTcp),
      _broadcastUdp(broadcastUdp), _broadcastTcp(broadcastTcp) {
    _scoreEntity = _reg.createEntity();
    _reg.addComponent<Position>(_scoreEntity, 0.f, 0.f);
    _reg.addComponent<Label>(
        _scoreEntity, std::string("Score: 0"),
        std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
        std::string("default_font"), Color(255, 255, 255), 0, true);
}

void RoomGameHandler::run() {
    LOG_INFO("Room game loop started");

    auto lastTime = std::chrono::steady_clock::now();
    constexpr float targetFrameTime = 1.0f / 30.0f; // 30 ticks per second

    while (_running) {
        auto currentTime = std::chrono::steady_clock::now();
        float deltaTime =
            std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Update game state
        updateGame(deltaTime);

        // Send position updates
        sendUpdatedPositionToAllPlayers();

        // Frame rate limiting
        auto frameEnd = std::chrono::steady_clock::now();
        float frameTime =
            std::chrono::duration<float>(frameEnd - currentTime).count();
        if (frameTime < targetFrameTime) {
            std::this_thread::sleep_for(
                std::chrono::duration<float>(targetFrameTime - frameTime));
        }
    }

    LOG_INFO("Room game loop stopped");
}

void RoomGameHandler::updateGame(float deltaTime) {
    std::lock_guard<std::mutex> lock(_regMutex); // Protect ECS access

    _enemySystem.update(_reg, deltaTime);

    for (const auto &[enemy, projectile] :
         _enemySystem.getNewProjectileEntitiesWithParent()) {
        sendNewProjectilesToAllPlayers(enemy, projectile);
    }
    for (const auto &projectile : _enemySystem.getProjectileColliding()) {
        sendDestroyedProjectileToAllPlayers(projectile);
        _reg.destroyEntity(projectile);
    }
    for (const auto &enemy : _enemySystem.getDeadEnemyEntities()) {
        sendDestroyedEnemyToAllPlayers(enemy);
    }

    initNewEnemyEntities(_enemySystem.getNewEnemyEntities());
    updateEnemyPosition(_enemySystem.getEnemyEntities());
    _movement.update(_reg, deltaTime);
    _statSystem.update(_reg, deltaTime);
}

void RoomGameHandler::onPlayerConnect(uint32_t playerId) {
    std::lock_guard<std::mutex> regLock(_regMutex); // Lock registry first
    std::lock_guard<std::mutex> lock(_playersMutex);

    Entity playerEntity = _reg.createEntity();
    _reg.addComponent<Position>(playerEntity, 0.f, 0.f);
    _reg.addComponent<Velocity>(playerEntity, 0.f, 0.f);
    _reg.addComponent<Stats>(playerEntity, 100, 100, 1, 0.f, 10, 1, 200);

    auto &factory = MessageFactory::getInstance();

    // Send all existing players' info to the new player
    for (const auto &[existingPlayerId, existingEntity] : _playerEntities) {
        Position &pos = _reg.getComponent<Position>(existingEntity);
        MessageData payload = factory.encodePlayerInfo(
            existingPlayerId, existingEntity, pos.x, pos.y);
        PreparedMessage msg = factory.createMessage(OpCode::PLAYER, payload);
        _sendTcp(playerId, msg);
        LOG_DEBUG("Sent existing player " + std::to_string(existingPlayerId) +
                  " info to new player " + std::to_string(playerId));
    }

    // Add new player to the map
    _playerEntities[playerId] = playerEntity;

    if (_playerEntities.size() >= 1 && !_gameStarted) {
        _gameStarted = true;
        LOG_INFO("Minimum players reached. Game started!");
        _enemySystem.startSpawning();
    }

    // Send the new player's info to all players (including themselves)
    Position &newPlayerPos = _reg.getComponent<Position>(playerEntity);
    MessageData payload = factory.encodePlayerInfo(
        playerId, playerEntity, newPlayerPos.x, newPlayerPos.y);
    PreparedMessage msg = factory.createMessage(OpCode::PLAYER, payload);
    for (const auto &[existingPlayerId, existingEntity] : _playerEntities) {
        _sendTcp(existingPlayerId, msg);
        LOG_DEBUG("Sent new player " + std::to_string(playerId) +
                  " info to player " + std::to_string(existingPlayerId));
    }

    LOG_INFO("Player " + std::to_string(playerId) +
             " entity created in room, synced with " +
             std::to_string(_playerEntities.size() - 1) + " other players");
}

void RoomGameHandler::onPlayerDisconnect(uint32_t playerId) {
    std::lock_guard<std::mutex> regLock(_regMutex); // Lock registry first
    std::lock_guard<std::mutex> lock(_playersMutex);

    auto it = _playerEntities.find(playerId);
    if (it != _playerEntities.end()) {
        _reg.destroyEntity(it->second);
        _playerEntities.erase(it);
        LOG_INFO("Player " + std::to_string(playerId) +
                 " entity destroyed in room");
    }
}

void RoomGameHandler::onPlayerMove(const RoomMoveData &moveData) {
    std::lock_guard<std::mutex> regLock(_regMutex); // Lock registry first
    std::lock_guard<std::mutex> lock(_playersMutex);

    auto it = _playerEntities.find(moveData.playerId);
    if (it == _playerEntities.end()) {
        LOG_WARN("Move received for unknown player: " +
                 std::to_string(moveData.playerId));
        return;
    }

    Entity entity = it->second;
    if (_reg.hasComponent<Velocity>(entity)) {
        auto &velocity = _reg.getComponent<Velocity>(entity);
        velocity.vx = moveData.vx;
        velocity.vy = moveData.vy;
        LOG_DEBUG("Player " + std::to_string(moveData.playerId) +
                  " velocity updated to (" + std::to_string(moveData.vx) +
                  ", " + std::to_string(moveData.vy) + ")");

        auto &position = _reg.getComponent<Position>(entity);
        LOG_DEBUG("Player " + std::to_string(moveData.playerId) +
                  " position is (" + std::to_string(position.x) + ", " +
                  std::to_string(position.y) + ")");
    }
}

void RoomGameHandler::onPlayerShoot(const RoomShootData &shootData) {
    std::lock_guard<std::mutex> regLock(_regMutex); // Lock registry first
    std::lock_guard<std::mutex> lock(_playersMutex);

    auto it = _playerEntities.find(shootData.playerId);
    if (it == _playerEntities.end()) {
        LOG_WARN("Shoot received for unknown player: " +
                 std::to_string(shootData.playerId));
        return;
    }

    Entity entity = it->second;
    if (!_reg.hasComponent<Stats>(entity)) {
        LOG_WARN("Shoot received for player without Stats component: " +
                 std::to_string(shootData.playerId));
        return;
    }

    auto &stats = _reg.getComponent<Stats>(entity);
    if (!stats.canAttack()) {
        LOG_DEBUG("Player " + std::to_string(shootData.playerId) +
                  " tried to shoot but is on cooldown");
        return;
    }

    Entity projectile = _reg.createEntity();
    _reg.addComponent<Position>(projectile, 0.f, 0.f);
    auto &pos = _reg.getComponent<Position>(entity);
    _reg.getComponent<Position>(projectile).y = pos.y + 30.f;
    _reg.getComponent<Position>(projectile).x = pos.x + 52.f;
    _reg.addComponent<Velocity>(projectile, 0.f, -400.f);
    _reg.addComponent<Projectile>(projectile, stats.attack_damage,
                                  std::string("player"));

    LOG_INFO("Player " + std::to_string(shootData.playerId) +
             " shot projectile " + std::to_string(projectile));

    stats.cooldown = 1.f / static_cast<float>(stats.attack_speed);

    sendNewProjectilesToAllPlayers(entity, projectile);
}

void RoomGameHandler::onPlayerLinked(uint32_t playerId) {
    std::lock_guard<std::mutex> regLock(_regMutex); // Lock registry first
    std::lock_guard<std::mutex> lock(_playersMutex);

    auto &factory = MessageFactory::getInstance();

    for (Entity enemyEntity : _enemySystem.getEnemyEntities()) {
        if (!_reg.hasComponent<Position>(enemyEntity))
            continue;

        Position &pos = _reg.getComponent<Position>(enemyEntity);
        MessageData payload =
            factory.encodeMessageEnemy(enemyEntity, pos.x, pos.y);
        PreparedMessage msg = factory.createMessage(OpCode::ENEMY, payload);
        _sendUdp(playerId, msg);
        LOG_DEBUG("Sent existing enemy " + std::to_string(enemyEntity) +
                  " to newly linked player " + std::to_string(playerId));
    }
}

bool RoomGameHandler::hasMinimumPlayers() const {
    std::lock_guard<std::mutex> lock(_playersMutex);
    return _playerEntities.size() >= 1;
}

void RoomGameHandler::sendUpdatedPositionToAllPlayers() {
    // Note: already holding _regMutex from the run() loop context
    std::lock_guard<std::mutex> lock(_playersMutex);

    for (const auto &[playerId, entity] : _playerEntities) {
        sendUpdatedPositionToPlayer(playerId);
    }
    for (const auto &[playerId, entity] : _playerEntities) {
        _wasMoving[entity] = _reg.hasComponent<Velocity>(entity) &&
                             (_reg.getComponent<Velocity>(entity).vx != 0.f ||
                              _reg.getComponent<Velocity>(entity).vy != 0.f);
    }
}

void RoomGameHandler::sendUpdatedPositionToPlayer(uint32_t playerId) {
    auto &factory = MessageFactory::getInstance();

    // Send player positions
    for (const auto &[pid, entity] : _playerEntities) {
        Position &pos = _reg.getComponent<Position>(entity);
        if (!_reg.hasComponent<Velocity>(entity))
            continue;

        bool isMoving = _reg.getComponent<Velocity>(entity).vx != 0.f ||
                        _reg.getComponent<Velocity>(entity).vy != 0.f;
        bool entityWasMoving = _wasMoving[entity];

        if (isMoving || entityWasMoving) {
            MessageData payload = factory.encodeMessageMove(
                EntityType::PLAYER, entity, pos.x, pos.y);
            PreparedMessage msg =
                factory.createMessage(OpCode::MOVE_SYNC, payload);
            _sendUdp(playerId, msg);
        }
    }
}

void RoomGameHandler::sendNewProjectilesToAllPlayers(Entity parentEntity,
                                                     Entity projectileEntity) {
    std::string ownerType = "player";
    if (_reg.hasComponent<Projectile>(projectileEntity)) {
        ownerType = _reg.getComponent<Projectile>(projectileEntity).ownerType;
    }

    MessageData payload = MessageFactory::getInstance().encodeMessageProjectile(
        projectileEntity, parentEntity, ownerType);
    PreparedMessage msg =
        MessageFactory::getInstance().createMessage(OpCode::SHOOT, payload);

    _broadcastUdp(msg);

    LOG_DEBUG("Sent projectile " + std::to_string(projectileEntity) +
              " from parent " + std::to_string(parentEntity) +
              " to all players");
}

void RoomGameHandler::sendDestroyedProjectileToAllPlayers(Entity projectile) {
    auto &factory = MessageFactory::getInstance();
    MessageData payload =
        factory.encodeMessageDeath(EntityType::PROJECTILE, projectile);
    PreparedMessage msg = factory.createMessage(OpCode::DEATH, payload);

    _broadcastUdp(msg);

    LOG_DEBUG("Sent DEATH for projectile " + std::to_string(projectile) +
              " to all players");
}

void RoomGameHandler::sendDestroyedEnemyToAllPlayers(Entity enemy) {
    auto &factory = MessageFactory::getInstance();
    MessageData payload = factory.encodeMessageDeath(EntityType::ENEMY, enemy);
    PreparedMessage msg = factory.createMessage(OpCode::DEATH, payload);

    _broadcastUdp(msg);

    LOG_DEBUG("Sent DEATH for enemy " + std::to_string(enemy) +
              " to all players");
}

void RoomGameHandler::sendDestroyedPlayerToAllPlayers(Entity player) {
    auto &factory = MessageFactory::getInstance();
    MessageData payload =
        factory.encodeMessageDeath(EntityType::PLAYER, player);
    PreparedMessage msg = factory.createMessage(OpCode::DEATH, payload);

    _broadcastUdp(msg);

    LOG_DEBUG("Sent DEATH for player " + std::to_string(player) +
              " to all players");
}

void RoomGameHandler::initNewEnemyEntities(
    const std::vector<Entity> &newEnemyEntities) {
    for (const auto &enemyEntity : newEnemyEntities) {
        if (!_reg.hasComponent<Position>(enemyEntity))
            continue;

        Position &pos = _reg.getComponent<Position>(enemyEntity);
        MessageData payload = MessageFactory::getInstance().encodeMessageEnemy(
            enemyEntity, pos.x, pos.y);
        PreparedMessage msg =
            MessageFactory::getInstance().createMessage(OpCode::ENEMY, payload);

        _broadcastUdp(msg);
        LOG_DEBUG("Sent new enemy entity " + std::to_string(enemyEntity) +
                  " to all players");
    }
}

void RoomGameHandler::updateEnemyPosition(
    const std::vector<Entity> &allEnemyEntities) {
    auto &factory = MessageFactory::getInstance();

    for (const auto &enemyEntity : allEnemyEntities) {
        if (!_reg.hasComponent<Position>(enemyEntity))
            continue;

        Position &pos = _reg.getComponent<Position>(enemyEntity);
        auto vector = _reg.hasComponent<Velocity>(enemyEntity)
                          ? _reg.getComponent<Velocity>(enemyEntity)
                          : Velocity{0.f, 0.f};
        if (vector.vx == 0.f && vector.vy == 0.f)
            continue; // Enemy is not moving, no update needed

        MessageData payload = factory.encodeMessageMove(
            EntityType::ENEMY, enemyEntity, pos.x, pos.y);
        PreparedMessage msg = factory.createMessage(OpCode::MOVE_SYNC, payload);

        _broadcastUdp(msg);
    }
}
