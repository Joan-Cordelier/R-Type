#pragma once

#include "Session/Player.hpp"
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

class GameHandler {

private:
    bool _gameStarted = false;
    bool _running = true;
    Registry reg;
    MovementSystem movement;
    StatSystem statsys;

    std::map<uint32_t, Entity> playerEntities;
    Entity ScoreEntity;
    int score = 0;

    MessageHandler messageHandler;

public:
    GameHandler(MessageHandler msgHandler);
    void gameLoop();
    void onPlayerConnect(Player player);
    void onPlayerDisconnect(Player player);
};