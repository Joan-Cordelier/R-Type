#include "GameHandler.hpp"
#include "../Logs/Logger.hpp"

#include <chrono>
#include <thread>

GameHandler::GameHandler(SessionManager& session, std::atomic<bool>& running)
    : _running(running)
    , _session(session)
    , _messageHandler(session, running)
{
    // Set up message handler callbacks
    _messageHandler.setOnPlayerConnect([this](const Player& player) {
        onPlayerConnect(player);
    });
    
    _messageHandler.setOnPlayerMove([this](const MoveData& moveData) {
        onPlayerMove(moveData);
    });
    
    // Set up session manager callback for disconnections
    _session.setOnPlayerDisconnect([this](const Player& player) {
        onPlayerDisconnect(player);
    });
    
    ScoreEntity = reg.createEntity();
    reg.addComponent<Position>(ScoreEntity, 0.f, 0.f);
    reg.addComponent<Label>(ScoreEntity, std::string("Score: 0"), std::string("font/josefin-sans/JosefinSans-Regular.ttf"), std::string("default_font"), Color(255, 255, 255), 0, true);
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
}

/// @brief send cur position of all entity containing a vector to a player
/// @param playerId the id of the player to send the position to
void GameHandler::sendUpdatedPositionToPlayer(uint32_t playerId)
{
    for (const auto& [pid, entity] : playerEntities) {
        Position& pos = reg.getComponent<Position>(entity);
        if (!reg.hasComponent<Velocity>(entity))
            continue;
        
        bool isMoving = reg.getComponent<Velocity>(entity).vx != 0.f || reg.getComponent<Velocity>(entity).vy != 0.f;
        bool entityWasMoving = wasMoving[entity];
        
        // Send update if moving OR if just stopped moving (one final update)
        if (isMoving || entityWasMoving) {
            MessageData payload = MessageFactory::getInstance().encodeMessageMovementPlayer(entity, pos.x, pos.y);
            PreparedMessage msg = MessageFactory::getInstance().createMessage(OpCode::MOVE, payload);
            _session.sendUdp(playerId, msg);
        }
        
        // Update movement state for next frame
        wasMoving[entity] = isMoving;
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
    movement.update(reg, deltaTime);
    statsys.update(reg, deltaTime);
}

void GameHandler::onPlayerConnect(const Player& player)
{
    Entity playerEntity = reg.createEntity();
    reg.addComponent<Position>(playerEntity, 0.f, 0.f);
    reg.addComponent<Velocity>(playerEntity, 0.f, 0.f);
    reg.addComponent<Stats>(playerEntity, 100, 100, 1, 0.f, 10, 1, 200);
    
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