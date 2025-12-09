#pragma once

#include "Session/Player.hpp"
#include "Session/SessionManager.hpp"
#include "Handler/MessageHandler.hpp"

#include "ecs/registry.hpp"
#include "ecs/systems/movement_system.hpp"
#include "ecs/systems/sprite_system.hpp"
#include "ecs/systems/button_system.hpp"
#include "ecs/systems/label_system.hpp"
#include "ecs/systems/spritesheet_system.hpp"
#include "ecs/systems/stat_system.hpp"

#include "../common/ecs/components/position.hpp"
#include "../common/ecs/components/velocity.hpp"
#include "../common/ecs/components/sprite.hpp"
#include "../common/ecs/components/button.hpp"
#include "../common/ecs/components/label.hpp"
#include "../common/ecs/components/stats.hpp"
#include "../common/ecs/components/spritesheet.hpp"

#include <map>
#include <atomic>

class GameHandler {

private:
    bool _gameStarted = false;
    std::atomic<bool>& _running;
    SessionManager& _session;
    MessageHandler _messageHandler;
    
    Registry reg;
    MovementSystem movement;
    StatSystem statsys;

    std::map<uint32_t, Entity> playerEntities;
    std::map<Entity, bool> wasMoving;  // Track if entity was moving last frame
    Entity ScoreEntity;
    int score = 0;

public:
    GameHandler(SessionManager& session, std::atomic<bool>& running);
    void run();

    void sendUpdatedPositionToAllPlayers();
    void sendUpdatedPositionToPlayer(uint32_t playerId);
    void sendNewProjectilesToAllPlayers();
    
private:
    void processMessages();
    void updateGame(float deltaTime);
    void onPlayerConnect(const Player& player);
    void onPlayerDisconnect(const Player& player);
    void onPlayerMove(const MoveData& moveData);
    void onPlayerShoot(const MoveData& shootData);
};