#include "ClientGameHandler.hpp"
#include "../common/Data/EntityType.hpp"

ClientGameHandler::ClientGameHandler()
{
    // Load resources
    _renderer.loadSpriteSheet("textures/ships/player_ship.png", "player_ship", 343, 383);
    _renderer.loadTexture("textures/play_button/default.png", "play_button");
    _renderer.loadSpriteSheet("textures/projectiles/projectile_player.png", "projectile_player", 16, 16);
    _renderer.loadFont("font/josefin-sans/JosefinSans-Regular.ttf", 40, "default_font");
    _renderer.loadTexture("textures/ships/enemy_ship.png", "enemy_ship");

    // Set up entities
    _reg.addComponent<Position>(start_button, 400.f, 300.f);
    _reg.addComponent<Sprite>(start_button, (std::string)"textures/play_button/default.png", (std::string)"play_button", 300, 150, 0, true);
    _reg.addComponent<Button>(start_button, (std::string)"start_game", 1, true);

    _reg.addComponent<Position>(label_input, 400.f, 200.f);
    _reg.addComponent<Label>(label_input, (std::string)"", (std::string)"font/josefin-sans/JosefinSans-Regular.ttf", (std::string)"default_font", Color(255, 255, 255), 0, true);

    // Register button handler
    _buttonsys.registerHandler("start_game", [&](Registry& r, Entity e) {
        ip_adress = _reg.getComponent<Label>(label_input).text;
        if (ip_adress == "") {
            ip_adress = "127.0.0.1";
        }
        if (_network.connect(ip_adress, 4789, 4790) != 0) {
            std::cerr << "Failed to connect to server" << std::endl;
            return;
        }
        _network.start();
        r.getComponent<Label>(label_input).visible = false;
        r.getComponent<Sprite>(e).visible = false;
        r.getComponent<Button>(e).enabled = false;

        MessageFactory& factory = MessageFactory::getInstance();
        PreparedMessage msg = factory.createMessage(OpCode::CONNECT, {});
        _network.sendTcp(msg);
    });
}

int ClientGameHandler::run()
{
    Uint64 last = SDL_GetPerformanceCounter();
    _input.setControlled(label_input);

    while (running) {
        _renderer.window.processSDLEvents();
        SDL_Event status = _renderer.window.pollEvent();
        if (status.type == SDL_QUIT)
            break;

        Uint64 now = SDL_GetPerformanceCounter();
        double dt = (double)(now - last) / SDL_GetPerformanceFrequency();
        animationClock += dt;
        last = now;

        handleMessages();

        _input.update(_reg, status, _network);

        _movement.update(_reg, static_cast<float>(dt));

        _buttonsys.update(_reg);

        _renderer.clear();

        _statsys.update(_reg, static_cast<float>(dt));

        _spritesys.render(_reg, [&](const SpriteSystem::TextureId& tid, int width, int height, int x, int y, int z) {
            _renderer.drawTexture(tid, RenderLayer::GAME, z, Rect{x, y, width, height});
        });

        _spritesheetsys.render(_reg, [&](const SpriteSheetSystem::TextureId& tid, int frameIndex, int width, int height, int x, int y, int z) {
            _renderer.drawFrame(tid, frameIndex, RenderLayer::GAME, z, Rect{x, y, width, height});
        }, animationClock);

        _labelsys.render(_reg, [&](const LabelSystem::TextId& tid, std::string& text, int x, int y, Color color) {
            _renderer.drawFont(tid, text, x, y, color, RenderLayer::OVERLAY, 0);
        });

        _renderer.render();
    }

    return 0;
}

void ClientGameHandler::cleanupServerEntity(Entity serverEntity)
{
    // Nettoyer dans playerEntities
    auto itPlayer = playerEntities.find(serverEntity);
    if (itPlayer != playerEntities.end()) {
        std::cout << "Warning: Cleaning up old player entity with serverEntity " << serverEntity << std::endl;
        _reg.destroyEntity(itPlayer->second);
        playerEntities.erase(itPlayer);
    }
    
    // Nettoyer dans enemyEntities
    auto itEnemy = enemyEntities.find(serverEntity);
    if (itEnemy != enemyEntities.end()) {
        std::cout << "Warning: Cleaning up old enemy entity with serverEntity " << serverEntity << std::endl;
        _reg.destroyEntity(itEnemy->second);
        enemyEntities.erase(itEnemy);
    }
    
    // Nettoyer dans projectileEntities
    auto itProj = projectileEntities.find(serverEntity);
    if (itProj != projectileEntities.end()) {
        std::cout << "Warning: Cleaning up old projectile entity with serverEntity " << serverEntity << std::endl;
        _reg.destroyEntity(itProj->second);
        projectileEntities.erase(itProj);
    }
}

void ClientGameHandler::handleMessages()
{
    std::optional<DecodedMessage> msg;
    while (_network.hasMessages()) {
        msg = _network.popMessage(Priority::CRITICAL);
        if (msg.has_value() == false) {
            msg = _network.popMessage(Priority::HIGH);
            if (msg.has_value() == false) {
                msg = _network.popMessage(Priority::MEDIUM);
                if (msg.has_value() == false) {
                    msg = _network.popMessage(Priority::LOW);
                    if (msg.has_value() == false) {
                        msg = _network.popMessage(Priority::ERROR);
                        if (msg.has_value() == false) {
                            break;
                        }
                    }
                }
            }
        }
        switch (msg->opCode) {
            case OpCode::CONNECT_ACK: {
                if (msg->data.size() >= 4) {
                    myPlayerId = 
                        (static_cast<uint32_t>(msg->data[0]) << 24) |
                        (static_cast<uint32_t>(msg->data[1]) << 16) |
                        (static_cast<uint32_t>(msg->data[2]) << 8) |
                        static_cast<uint32_t>(msg->data[3]);
                    std::cout << "Received playerId: " << myPlayerId << std::endl;

                    MessageFactory& factory = MessageFactory::getInstance();
                    std::vector<uint8_t> linkPayload;
                    linkPayload.push_back(static_cast<uint8_t>((myPlayerId >> 24) & 0xFF));
                    linkPayload.push_back(static_cast<uint8_t>((myPlayerId >> 16) & 0xFF));
                    linkPayload.push_back(static_cast<uint8_t>((myPlayerId >> 8) & 0xFF));
                    linkPayload.push_back(static_cast<uint8_t>(myPlayerId & 0xFF));
                    PreparedMessage linkMsg = factory.createMessage(OpCode::LINK, linkPayload);
                    _network.sendUdp(linkMsg);
                    
                    struct sockaddr_in localAddr;
                    socklen_t addrLen = sizeof(localAddr);
                    int udpFd = _network.getUdpClient().getSocketFd();
                    if (getsockname(udpFd, (struct sockaddr*)&localAddr, &addrLen) == 0) {
                        uint16_t localPort = ntohs(localAddr.sin_port);
                        std::cout << "Sent LINK message via UDP, listening on port: " << localPort << std::endl;
                    } else {
                        std::cout << "Sent LINK message via UDP" << std::endl;
                    }
                    
                    for (auto& pendingMsg : pendingPlayerPackets) {
                        handlePlayerPacket(pendingMsg);
                    }
                    pendingPlayerPackets.clear();
                }
                break;
            }
            case OpCode::PLAYER: {
                handlePlayerPacket(*msg);
                break;
            }
            case OpCode::MOVE_SYNC: {
                if (msg->data.size() >= 13) {
                    EntityType type = static_cast<EntityType>(msg->data[0]);
                    
                    Entity serverEntity = 
                        (static_cast<Entity>(msg->data[1]) << 24) |
                        (static_cast<Entity>(msg->data[2]) << 16) |
                        (static_cast<Entity>(msg->data[3]) << 8) |
                        static_cast<Entity>(msg->data[4]);
                    
                    float x = *reinterpret_cast<const float*>(&msg->data[5]);
                    float y = *reinterpret_cast<const float*>(&msg->data[9]);
                    
                    switch (type) {
                        case EntityType::PLAYER: {
                            auto itPlayer = playerEntities.find(serverEntity);
                            if (itPlayer != playerEntities.end()) {
                                Entity localEntity = itPlayer->second;
                                
                                float prevX = _reg.getComponent<Position>(localEntity).x;
                                if (prevX > x) {
                                    _reg.getComponent<SpriteSheets>(localEntity).frameIndex = 0;
                                } else if (prevX < x) {
                                    _reg.getComponent<SpriteSheets>(localEntity).frameIndex = 2;
                                } else {
                                    _reg.getComponent<SpriteSheets>(localEntity).frameIndex = 1;
                                }
                                
                                _reg.getComponent<Position>(localEntity).x = x;
                                _reg.getComponent<Position>(localEntity).y = y;
                            }
                            break;
                        }
                        case EntityType::ENEMY: {
                            auto itEnemy = enemyEntities.find(serverEntity);
                            if (itEnemy != enemyEntities.end()) {
                                Entity localEntity = itEnemy->second;
                                _reg.getComponent<Position>(localEntity).x = x;
                                _reg.getComponent<Position>(localEntity).y = y;
                            }
                            break;
                        }
                        case EntityType::PROJECTILE: {
                            auto itProj = projectileEntities.find(serverEntity);
                            if (itProj != projectileEntities.end()) {
                                Entity localEntity = itProj->second;
                                _reg.getComponent<Position>(localEntity).x = x;
                                _reg.getComponent<Position>(localEntity).y = y;
                            }
                            break;
                        }
                    }
                }
                break;
            }
            case OpCode::SHOOT: {
                std::cout << "Received SHOOT message, size=" << msg->data.size() << std::endl;
                if (msg->data.size() >= 18) {
                    Entity serverProjectileEntity = 
                        (static_cast<Entity>(msg->data[0]) << 24) |
                        (static_cast<Entity>(msg->data[1]) << 16) |
                        (static_cast<Entity>(msg->data[2]) << 8) |
                        static_cast<Entity>(msg->data[3]);
                    
                    Entity serverParentEntity = 
                        (static_cast<Entity>(msg->data[4]) << 24) |
                        (static_cast<Entity>(msg->data[5]) << 16) |
                        (static_cast<Entity>(msg->data[6]) << 8) |
                        static_cast<Entity>(msg->data[7]);
                    
                    std::string ownerType;
                    for (size_t i = 8; i < 18; ++i) {
                        ownerType += static_cast<char>(msg->data[i]);
                    }
                    ownerType.erase(std::find(ownerType.begin(), ownerType.end(), '\0'), ownerType.end());

                    auto itProj = projectileEntities.find(serverProjectileEntity);
                    if (itProj != projectileEntities.end()) {
                        break;
                    }

                    // Nettoyer les anciennes références AVANT de créer le projectile
                    cleanupServerEntity(serverProjectileEntity);

                    if (ownerType == "player") {
                        auto it = playerEntities.find(serverParentEntity);
                        if (it != playerEntities.end()) {
                            Entity localParent = it->second;
                            Entity projectile = _reg.createEntity();
                            Position &pos = _reg.getComponent<Position>(localParent);
                            _reg.addComponent<Position>(projectile, pos.x + 52.f, pos.y + 30.f);
                            _reg.addComponent<Velocity>(projectile, 0.f, -400.f);
                            _reg.addComponent<SpriteSheets>(projectile, std::string("textures/projectiles/projectile_player.png"), std::string("projectile_player"), 16, 16, 0, 4, 0, true, true);
                            
                            projectileEntities[serverProjectileEntity] = projectile;
                            std::cout << "Created player projectile (serverId: " << serverProjectileEntity << ", localId: " << projectile << ")" << std::endl;
                        }
                    } else if (ownerType == "enemy") {
                        auto it = enemyEntities.find(serverParentEntity);
                        if (it != enemyEntities.end()) {
                            Entity localParent = it->second;
                            Entity projectile = _reg.createEntity();
                            Position &pos = _reg.getComponent<Position>(localParent);
                            _reg.addComponent<Position>(projectile, pos.x + 18.f, pos.y + 50.f);
                            _reg.addComponent<Velocity>(projectile, 0.f, 200.f);
                            _reg.addComponent<SpriteSheets>(projectile, std::string("textures/projectiles/projectile_player.png"), std::string("projectile_player"), 16, 16, 0, 4, 0, true, true);
                            
                            projectileEntities[serverProjectileEntity] = projectile;
                            std::cout << "Created enemy projectile (serverId: " << serverProjectileEntity << ", localId: " << projectile << ")" << std::endl;
                        }
                    } else {
                        std::cout << "Invalid ownerType in SHOOT message: " << ownerType << std::endl;
                    }
                }
                break;
            }
            case OpCode::ENEMY: {
                if (msg->data.size() >= 12) {
                    Entity serverEntity = 
                        (static_cast<Entity>(msg->data[0]) << 24) |
                        (static_cast<Entity>(msg->data[1]) << 16) |
                        (static_cast<Entity>(msg->data[2]) << 8) |
                        static_cast<Entity>(msg->data[3]);
                    
                    float x = *reinterpret_cast<const float*>(&msg->data[4]);
                    float y = *reinterpret_cast<const float*>(&msg->data[8]);

                    auto it = enemyEntities.find(serverEntity);
                    if (it == enemyEntities.end()) {
                        cleanupServerEntity(serverEntity);
                        
                        Entity localEntity = _reg.createEntity();
                        _reg.addComponent<Position>(localEntity, x, y);
                        _reg.addComponent<Velocity>(localEntity, 0.f, 0.f);
                        _reg.addComponent<Sprite>(localEntity, std::string("textures/ships/enemy_ship.png"), std::string("enemy_ship"), 50, 50, 0, true);

                        enemyEntities[serverEntity] = localEntity;
                        std::cout << "Created enemy entity (serverId: " << serverEntity << ", localId: " << localEntity << ") at (" << x << ", " << y << ")" << std::endl;
                    } else {
                        Entity localEntity = it->second;
                        _reg.getComponent<Position>(localEntity).x = x;
                        _reg.getComponent<Position>(localEntity).y = y;
                    }
                }
                break;
            }
            case OpCode::DEATH: {
                if (msg->data.size() >= 5) {
                    EntityType type = static_cast<EntityType>(msg->data[0]);
                    
                    Entity serverEntity = 
                        (static_cast<Entity>(msg->data[1]) << 24) |
                        (static_cast<Entity>(msg->data[2]) << 16) |
                        (static_cast<Entity>(msg->data[3]) << 8) |
                        static_cast<Entity>(msg->data[4]);
                
                    switch (type) {
                        case EntityType::PROJECTILE: {
                            auto itProjectile = projectileEntities.find(serverEntity);
                            if (itProjectile != projectileEntities.end()) {
                                _reg.destroyEntity(itProjectile->second);
                                projectileEntities.erase(itProjectile);
                                std::cout << "Projectile " << serverEntity << " destroyed" << std::endl;
                            }
                            break;
                        }
                        case EntityType::ENEMY: {
                            auto itEnemy = enemyEntities.find(serverEntity);
                            if (itEnemy != enemyEntities.end()) {
                                _reg.destroyEntity(itEnemy->second);
                                enemyEntities.erase(itEnemy);
                                std::cout << "Enemy " << serverEntity << " destroyed" << std::endl;
                            }
                            break;
                        }
                        case EntityType::PLAYER: {
                            auto itPlayer = playerEntities.find(serverEntity);
                            if (itPlayer != playerEntities.end()) {
                                _reg.destroyEntity(itPlayer->second);
                                playerEntities.erase(itPlayer);
                                std::cout << "Player " << serverEntity << " destroyed" << std::endl;
                            }
                            break;
                        }
                    }
                }
                break;
            }
            default:
                break;
        }
    } 
}

void ClientGameHandler::handlePlayerPacket(const DecodedMessage& msg)
{
    if (msg.data.size() < 16) {
        return;
    }
    if (myPlayerId == 0) {
        pendingPlayerPackets.push_back(msg);
        return;
    }

    uint32_t playerId = 
        (static_cast<uint32_t>(msg.data[0]) << 24) |
        (static_cast<uint32_t>(msg.data[1]) << 16) |
        (static_cast<uint32_t>(msg.data[2]) << 8) |
        static_cast<uint32_t>(msg.data[3]);
    
    Entity serverEntity =
        (static_cast<Entity>(msg.data[4]) << 24) |
        (static_cast<Entity>(msg.data[5]) << 16) |
        (static_cast<Entity>(msg.data[6]) << 8) |
        static_cast<Entity>(msg.data[7]);

    float x = *reinterpret_cast<const float*>(&msg.data[8]);
    float y = *reinterpret_cast<const float*>(&msg.data[12]);

    auto it = playerEntities.find(serverEntity);
    if (it == playerEntities.end()) {
        // Nettoyer les anciennes références AVANT de créer la nouvelle entité
        cleanupServerEntity(serverEntity);
        
        Entity localEntity = _reg.createEntity();
        _reg.addComponent<Position>(localEntity, x, y);
        _reg.addComponent<Velocity>(localEntity, 0.f, 0.f);
        _reg.addComponent<SpriteSheets>(localEntity, std::string("textures/ships/player_ship.png"), std::string("player_ship"), 120, 130, 1, 3, 0, true, false);
        _reg.addComponent<Stats>(localEntity, 100, 100, 1, 0.f, 10, 1, 200);
                        
        playerEntities[serverEntity] = localEntity;

        if (playerId == myPlayerId) {
            myEntity = localEntity;
            _input.setControlled(localEntity);
            std::cout << "Created my player entity (serverId: " << serverEntity << ", localId: " << localEntity << ") at (" << x << ", " << y << ")" << std::endl;
        } else {
            std::cout << "Created other player entity (serverId: " << serverEntity << ", localId: " << localEntity << ") at (" << x << ", " << y << ")" << std::endl;
        }
    } else {
        Entity localEntity = it->second;
        _reg.getComponent<Position>(localEntity).x = x;
        _reg.getComponent<Position>(localEntity).y = y;
    }
}
