#pragma once

#include "graphic/Renderer.hpp"
#include "input_system.hpp"
#include "Network/NetworkManager.hpp"
#include "../common/Data/MessageFactory.hpp"

#include "../common/ecs/registry.hpp"
#include "../common/ecs/ecs_system.hpp"
#include "../common/ecs/ecs_components.hpp"

#include <functional>
#include <map>
#include <unordered_map>
#include <vector>
#include <SDL2/SDL.h>
#include <arpa/inet.h>
#include <sys/socket.h>

class ClientGameHandler {
private:
    Renderer _renderer;
    Registry _reg;
    MovementSystem _movement;
    SpriteSystem _spritesys;
    ButtonSystem _buttonsys;
    LabelSystem _labelsys;
    SpriteSheetSystem _spritesheetsys;
    StatSystem _statsys;
    InputSystem _input;

    NetworkManager _network;

    std::map<Entity, Entity> playerEntities;
    std::vector<DecodedMessage> pendingPlayerPackets;

    double animationClock = 0.0;
    bool running = true;

    std::string ip_adress = "127.0.0.1";

    // menu Entities
    Entity start_button = _reg.createEntity();
    Entity label_input = _reg.createEntity();

    void handleMessages();
    void handlePlayerPacket(const DecodedMessage& msg);
public:
    uint32_t myPlayerId = 0;
    Entity myEntity = 0;
    ClientGameHandler();
    ~ClientGameHandler() = default;
    int run();
};