#include "ClientGameHandler.hpp"
#include "../common/Data/EntityType.hpp"
#include "../common/ecs/components/weapon.hpp"
#include "../common/ecs/components/enemy.hpp"
#include <cmath>
#include <cstring>
#include <map>
#include <set>
#include <sstream>

ClientGameHandler::ClientGameHandler(bool debugMode)
    : _settingsMenu(_reg, _keybindsManager), _lobbyMenu(_reg), _createRoomMenu(_reg), _debugMode(debugMode) {
    // Load config (try local, then ../ for build dir)
    if (!_config.loadFromFile("yaml/main_loop.yaml")) {
        // Only try parent directory if first attempt failed
        _config.loadFromFile("../yaml/main_loop.yaml");
    }

    // Load upgrades
    if (!_config.loadUpgradesFromFile("yaml/upgrades.yaml")) {
        _config.loadUpgradesFromFile("../yaml/upgrades.yaml");
    }

    // Load boss configurations
    std::string bossDir = "yaml/enemies";
    if (!_config.loadBossesFromDirectory(bossDir)) {
        _config.loadBossesFromDirectory("../" + bossDir);
    }

    if (_debugMode) {
        auto &hb = _config.getPlayerConfig().hitbox;
        std::cout << "[DEBUG] Loaded Hitbox Config: "
                  << "W=" << hb.width << " H=" << hb.height << " OffX=" << hb.offset_x
                  << " OffY=" << hb.offset_y << std::endl;
    }

    // Load resources
    _renderer.loadSpriteSheet("textures/ships/player_ship.png", "player_ship", 343, 383);
    _renderer.loadSpriteSheet("textures/ships/player_ship_blue.png", "player_ship_blue", 343, 383);
    _renderer.loadSpriteSheet("textures/ships/player_ship_green.png", "player_ship_green", 343,
                              383);
    _renderer.loadSpriteSheet("textures/ships/player_ship_yellow.png", "player_ship_yellow", 343,
                              383);

    _renderer.loadTexture("textures/play_button/default.png", "play_button");
    _renderer.loadTexture("textures/button/square_button.png", "square_button");
    _renderer.loadTexture("textures/upgrades/border.png", "upgrade_border");
    _renderer.loadSpriteSheet("textures/projectiles/projectile_player.png", "projectile_player", 16,
                              16);
    _renderer.loadFont("font/josefin-sans/JosefinSans-Regular.ttf", 40, "default_font");
    _renderer.loadFont("font/josefin-sans/JosefinSans-Regular.ttf", 25, "default_font_small");
    _renderer.loadFont("font/josefin-sans/JosefinSans-Regular.ttf", 14, "default_font_medium");
    _renderer.loadFont("font/josefin-sans/JosefinSans-Regular.ttf", 15, "default_font_tiny");
    _renderer.loadTexture("textures/ships/enemy_ship.png", "enemy_ship");
    // Dynamic Boss Loading
    const auto &bosses = _config.getBosses();
    if (bosses.empty()) {
        std::cout << "[WARNING] No boss configurations loaded for client." << std::endl;
        // Fallback just in case
        _renderer.loadSpriteSheet("textures/ships/bosses/Obelisk.png", "Ancient Obelisk", 240, 160);
    } else {
        for (const auto &[id, config] : bosses) {
            std::string texturePath = config.visuals.texture_path;
            int w = (int)config.visuals.width;
            int h = (int)config.visuals.height;

            if (config.visuals.animations.count("idle")) {
                w = config.visuals.animations.at("idle").width;
                h = config.visuals.animations.at("idle").height;
            }
            std::cout << "[INFO] Loading Boss Sprite: " << config.name << " (" << texturePath
                      << ") " << w << "x" << h << std::endl;
            // Ensure texture path is relative to what renderer expects if needed
            _renderer.loadSpriteSheet(texturePath, config.name, w, h);
        }
    }

    _renderer.loadSpriteSheet("textures/settingmenu/colorblindbtn.png", "daltonian_btn", 401, 108);
    _renderer.loadTexture("textures/settingmenu/bg.png", "settings_bg");
    _renderer.loadTexture("textures/bg.png", "background_game");
    _renderer.loadSpriteSheet("textures/settingmenu/Keybinds.png", "keybinds", 16, 16);
    _renderer.loadTexture("textures/vaisseau.png", "drone");

    // Set up entities
    _reg.addComponent<Position>(background, 0.f, 0.f);
    _reg.addComponent<Sprite>(background, (std::string) "textures/bg.png",
                              (std::string) "background_game", 1080, 720, -10, 0.f, 0.f, true);

    _reg.addComponent<Position>(start_button, 400.f, 300.f);
    _reg.addComponent<Sprite>(start_button, (std::string) "textures/play_button/default.png",
                              (std::string) "play_button", 300, 150, 0, 0.f, 0.f, true);
    _reg.addComponent<Button>(start_button, (std::string) "start_game", 1, true);

    _reg.addComponent<Position>(label_input, 400.f, 200.f);
    _reg.addComponent<Label>(label_input, (std::string) "",
                             (std::string) "font/josefin-sans/JosefinSans-Regular.ttf",
                             (std::string) "default_font", Color(255, 255, 255), 0, true);

    // Register button handler
    _buttonsys.registerHandler("start_game", [&](Registry &r, Entity e) {
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

        MessageFactory &factory = MessageFactory::getInstance();
        PreparedMessage msg = factory.createMessage(OpCode::CONNECT, {});
        _network.sendTcp(msg);

        // Will transition to LOBBY state after CONNECT_ACK
    });

    // Initialize lobby menu (after textures are loaded)
    _lobbyMenu.init();
    _createRoomMenu.init();

    // Setup lobby menu and its callbacks
    _lobbyMenu.setup(_buttonsys);
    _createRoomMenu.setup(_buttonsys);
    setupLobbyCallbacks();

    _settingsMenu.setup(_reg, _slidersys, _buttonsys);

    _weaponsys.setIsServer(false);
}

int ClientGameHandler::run() {
    std::map<Entity, float> voidZoneTimers;
    Uint64 last = SDL_GetPerformanceCounter();
    _input.setControlled(label_input, _keybindsManager);
    _audioManager.playMusic(_config.getAudioConfig().level_music);

    while (running) {
        _renderer.window.processSDLEvents();
        SDL_Event status = _renderer.window.pollEvent();
        if (status.type == SDL_QUIT)
            break;

        // Handle ESC key for settings menu
        if (status.type == SDL_KEYDOWN && status.key.keysym.sym == SDLK_ESCAPE) {
            std::cout << "toggling settings menu" << std::endl;
            toggleSettingsMenu();
        }
        _renderer.setDaltonianMode(_settingsMenu.getCurrentDaltonianMode(),
                                   _settingsMenu.getDaltonianSliderValue(_reg));

        Uint64 now = SDL_GetPerformanceCounter();
        double dt = (double)(now - last) / SDL_GetPerformanceFrequency();
        animationClock += dt;
        last = now;

        handleMessages();

        _weaponsys.update(_reg, static_cast<float>(dt));
        _input.update(_reg, status, _network);

        _movement.update(_reg, static_cast<float>(dt));

        // Clean up out-of-bounds projectiles
        for (auto it = projectileEntities.begin(); it != projectileEntities.end();) {
            Entity localEntity = it->second;
            if (_reg.hasComponent<Position>(localEntity)) {
                Position &pos = _reg.getComponent<Position>(localEntity);
                // Check if projectile is out of bounds
                if (pos.x < -100 || pos.x > 1920 || pos.y < -100 || pos.y > 1080) {
                    if (_debugMode) {
                        std::cout << "Cleaning up out-of-bounds projectile (localId: "
                                  << localEntity << ")" << std::endl;
                    }
                    _reg.destroyEntity(localEntity);
                    it = projectileEntities.erase(it);
                    continue;
                }
            }
            ++it;
        }

        _buttonsys.update(_reg);
        _slidersys.update(_reg);
        _audioManager.setMusicVolume(static_cast<int>(_settingsMenu.getMusicVolumeSliderValue(_reg)));
        _audioManager.setSoundVolume(static_cast<int>(_settingsMenu.getSoundVolumeSliderValue(_reg)));

        _renderer.clear();

        _statsys.update(_reg, static_cast<float>(dt));

        _spritesys.render(_reg, [&](const SpriteSystem::TextureId &tid, int width, int height,
                                    int x, int y, int z) {
            _renderer.drawTexture(tid, RenderLayer::GAME, z, Rect{x, y, width, height});
        });

        _spritesheetsys.render(
            _reg,
            [&](const SpriteSheetSystem::TextureId &tid, int frameIndex, int width, int height,
                int x, int y, int z) {
                _renderer.drawFrame(tid, frameIndex, RenderLayer::GAME, z,
                                    Rect{x, y, width, height});
            },
            animationClock);

        if (_debugMode) {
            // Draw hitboxes for players
            const auto &playerConf = _config.getPlayerConfig();
            for (auto const &[serverEntity, localEntity] : playerEntities) {
                if (_reg.hasComponent<Position>(localEntity)) {
                    Position &pos = _reg.getComponent<Position>(localEntity);

                    Color boxColor =
                        (localEntity == myEntity) ? Color{0, 255, 0, 255} : Color{0, 255, 255, 255};

                    // Position (x, y) is the top-left corner of the sprite
                    // Hitbox position = Sprite top-left + offset
                    float hitboxX = pos.x + playerConf.hitbox.offset_x;
                    float hitboxY = pos.y + playerConf.hitbox.offset_y;

                    _renderer.drawRect(Rect{(int)hitboxX, (int)hitboxY,
                                            (int)playerConf.hitbox.width,
                                            (int)playerConf.hitbox.height},
                                       boxColor, RenderLayer::OVERLAY, 100);
                }
            }

            // Draw hitboxes for enemies
            for (auto const &[serverEntity, localEntity] : enemyEntities) {
                if (_reg.hasComponent<Position>(localEntity)) {
                    Position &pos = _reg.getComponent<Position>(localEntity);
                    if (_reg.hasComponent<Sprite>(localEntity)) {
                        Sprite &sprite = _reg.getComponent<Sprite>(localEntity);
                        // Enemies usually have 50x50 or 60x60 depending on type, but for
                        // simple visualization using sprite size + red box
                        _renderer.drawRect(
                            Rect{(int)pos.x, (int)pos.y, sprite.width, sprite.height},
                            Color{255, 0, 0, 255}, RenderLayer::OVERLAY, 100);
                    } else if (_reg.hasComponent<SpriteSheets>(localEntity)) {
                        SpriteSheets &sprite = _reg.getComponent<SpriteSheets>(localEntity);
                        _renderer.drawRect(
                            Rect{(int)pos.x, (int)pos.y, sprite.width, sprite.height},
                            Color{255, 0, 0, 255}, RenderLayer::OVERLAY, 100);
                    }
                }
            }

            // Draw hitboxes for projectiles
            for (auto const &[serverEntity, localEntity] : projectileEntities) {
                if (_reg.hasComponent<Position>(localEntity)) {
                    Position &pos = _reg.getComponent<Position>(localEntity);
                    // Projectiles roughly 10x10 or 16x16
                    _renderer.drawRect(Rect{(int)pos.x, (int)pos.y, 16, 16},
                                       Color{255, 255, 0, 255}, RenderLayer::OVERLAY, 100);
                }
            }
        }

        _labelsys.render(_reg, [&](const LabelSystem::TextId &tid, std::string &text, int x, int y,
                                   Color color) {
            _renderer.drawFontAndCache(tid, text, x, y, color, RenderLayer::OVERLAY, 0);
        });

        _slidersys.render(_reg, _renderer);

        // Custom render for Void Zones
        std::set<Entity> currentVoidZones;
        auto spriteArr = _reg.componentArray<Sprite>();
        auto posArr = _reg.componentArray<Position>();

        if (spriteArr && posArr) {
            for (auto e : spriteArr->entities()) {
                if (!spriteArr->has(e))
                    continue;

                Sprite &sp = spriteArr->get(e);
                if (sp.textureIndex == "obelisk_void") {
                    currentVoidZones.insert(e);

                    if (voidZoneTimers.find(e) == voidZoneTimers.end()) {
                        voidZoneTimers[e] = 0.0f;
                    }
                    voidZoneTimers[e] += static_cast<float>(dt);

                    if (posArr->has(e)) {
                        Position &p = posArr->get(e);

                        float t = voidZoneTimers[e];
                        float maxTime = 1.5f;
                        float ratio = t / maxTime;
                        if (ratio > 1.0f)
                            ratio = 1.0f;

                        uint8_t alpha = static_cast<uint8_t>(50 + ratio * 150);

                        if (t > 1.5f) {
                            float flash = std::abs(std::sin((t - 1.5f) * 10.0f));
                            alpha = 200 + static_cast<uint8_t>(flash * 55);
                        }

                        _renderer.drawCircle((int)p.x + 50, (int)p.y + 50, 50,
                                             Color(255, 0, 0, alpha), RenderLayer::GAME, 10, true);
                        _renderer.drawCircle((int)p.x + 50, (int)p.y + 50, 50,
                                             Color(255, 0, 0, 255), RenderLayer::GAME, 11, false);

                        sp.visible = false;
                    }
                }
            }
        }

        // Cleanup timers
        for (auto it = voidZoneTimers.begin(); it != voidZoneTimers.end();) {
            if (currentVoidZones.find(it->first) == currentVoidZones.end()) {
                it = voidZoneTimers.erase(it);
            } else {
                ++it;
            }
        }

        // Draw Player Life Bar
        if (_reg.hasComponent<Stats>(myEntity)) {
            const auto &stats = _reg.getComponent<Stats>(myEntity);
            if (stats.maxHp > 0) {
                float healthPct = static_cast<float>(stats.hp) / static_cast<float>(stats.maxHp);
                if (healthPct < 0.0f)
                    healthPct = 0.0f;
                if (healthPct > 1.0f)
                    healthPct = 1.0f;

                int windowWidth = 1080;
                int windowHeight = 720;
                int barHeight = 20;
                int barWidth = static_cast<int>(windowWidth * healthPct);

                _renderer.drawRect(Rect{0, windowHeight - barHeight, barWidth, barHeight},
                                   Color{255, 0, 0, 255}, RenderLayer::OVERLAY, 200, true);
            }
        }

        _renderer.render();
    }

    return 0;
}

void ClientGameHandler::cleanupServerEntity(Entity serverEntity) {
    // Clean up from players
    auto itPlayer = playerEntities.find(serverEntity);
    if (itPlayer != playerEntities.end()) {
        std::cout << "Warning: Cleaning up old player entity with serverEntity " << serverEntity
                  << std::endl;
        _reg.destroyEntity(itPlayer->second);
        playerEntities.erase(itPlayer);
    }

    // Clean up from enemies
    auto itEnemy = enemyEntities.find(serverEntity);
    if (itEnemy != enemyEntities.end()) {
        std::cout << "Warning: Cleaning up old enemy entity with serverEntity " << serverEntity
                  << std::endl;
        _reg.destroyEntity(itEnemy->second);
        enemyEntities.erase(itEnemy);
    }

    // Clean up from projectiles
    auto itProj = projectileEntities.find(serverEntity);
    if (itProj != projectileEntities.end()) {
        // Warning log might be noisy for projectiles but useful for debugging this
        // issue
        std::cout << "Cleaning up old projectile entity with serverEntity " << serverEntity
                  << std::endl;
        _reg.destroyEntity(itProj->second);
        projectileEntities.erase(itProj);
    }

    // Clean up from companions
    auto itComp = companionEntities.find(serverEntity);
    if (itComp != companionEntities.end()) {
        std::cout << "Warning: Cleaning up old companion entity with serverEntity " << serverEntity
                  << std::endl;
        _reg.destroyEntity(itComp->second);
        companionEntities.erase(itComp);
    }
}

void ClientGameHandler::handleMessages() {
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
        std::cout << "[DEBUG] Received message OpCode: " << static_cast<int>(msg->opCode)
                  << " Size: " << msg->data.size() << std::endl;
        switch (msg->opCode) {
        case OpCode::CONNECT_ACK: {
            if (msg->data.size() >= 4) {
                myPlayerId = (static_cast<uint32_t>(msg->data[0]) << 24) |
                             (static_cast<uint32_t>(msg->data[1]) << 16) |
                             (static_cast<uint32_t>(msg->data[2]) << 8) |
                             static_cast<uint32_t>(msg->data[3]);
                std::cout << "Received playerId: " << myPlayerId << std::endl;

                MessageFactory &factory = MessageFactory::getInstance();
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
                if (getsockname(udpFd, (struct sockaddr *)&localAddr, &addrLen) == 0) {
                    uint16_t localPort = ntohs(localAddr.sin_port);
                    std::cout << "Sent LINK message via UDP, listening on port: " << localPort
                              << std::endl;
                } else {
                    std::cout << "Sent LINK message via UDP" << std::endl;
                }

                for (auto &pendingMsg : pendingPlayerPackets) {
                    handlePlayerPacket(pendingMsg);
                }
                pendingPlayerPackets.clear();

                // Transition to lobby state and show lobby menu
                _gameState = GameState::LOBBY;
                _lobbyMenu.show();
                requestRoomList();
            }
            break;
        }

        case OpCode::ROOM_LIST: {
            std::cout << "Received Room List (Size: " << msg->data.size() << ")" << std::endl;
            std::vector<RoomInfo> rooms;

            if (msg->data.size() >= 1) {
                uint8_t count = msg->data[0];
                std::cout << "Room count: " << (int)count << std::endl;
                size_t offset = 1;

                for (uint8_t i = 0; i < count; ++i) {
                    if (offset + 5 > msg->data.size())
                        break;

                    uint32_t rId = (static_cast<uint32_t>(msg->data[offset]) << 24) |
                                   (static_cast<uint32_t>(msg->data[offset + 1]) << 16) |
                                   (static_cast<uint32_t>(msg->data[offset + 2]) << 8) |
                                   static_cast<uint32_t>(msg->data[offset + 3]);
                    uint8_t pCount = msg->data[offset + 4];
                    offset += 5;

                    std::cout << "Room " << rId << " (" << (int)pCount << "/4)" << std::endl;

                    RoomInfo info;
                    info.id = rId;
                    info.playerCount = pCount;
                    info.maxPlayers = 4;
                    rooms.push_back(info);

                    // Register join handler for this room
                    registerJoinHandler(rId);
                }
            } else {
                std::cout << "Empty room list data" << std::endl;
            }

            std::cout << "Updating lobby menu with " << rooms.size() << " rooms" << std::endl;
            // Update lobby menu with room list
            _lobbyMenu.updateRoomList(rooms);
        } break;
        case OpCode::ROOM_CREATED: {
            if (msg->data.size() >= 4) {
                uint32_t roomId = (static_cast<uint32_t>(msg->data[0]) << 24) |
                                  (static_cast<uint32_t>(msg->data[1]) << 16) |
                                  (static_cast<uint32_t>(msg->data[2]) << 8) |
                                  static_cast<uint32_t>(msg->data[3]);

                std::cout << "Room created with ID: " << roomId << ". Refreshing room list..."
                          << std::endl;

                // Register join handler for this new room
                registerJoinHandler(roomId);

                // Refresh the room list so the player can see and join the new room
                requestRoomList();
            }
            break;
        }
        case OpCode::JOIN_ACK: {
            if (msg->data.size() >= 5) {
                uint32_t roomId = (static_cast<uint32_t>(msg->data[0]) << 24) |
                                  (static_cast<uint32_t>(msg->data[1]) << 16) |
                                  (static_cast<uint32_t>(msg->data[2]) << 8) |
                                  static_cast<uint32_t>(msg->data[3]);
                bool success = msg->data[4] != 0;

                if (success) {
                    std::cout << "Successfully joined room " << roomId << std::endl;
                    _joinedRoom = true;
                    _gameState = GameState::IN_GAME;
                    _lobbyMenu.hide();
                } else {
                    std::cerr << "Failed to join room " << roomId << std::endl;
                    // Refresh room list to see updated availability
                    requestRoomList();
                }
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

                Entity serverEntity = (static_cast<Entity>(msg->data[1]) << 24) |
                                      (static_cast<Entity>(msg->data[2]) << 16) |
                                      (static_cast<Entity>(msg->data[3]) << 8) |
                                      static_cast<Entity>(msg->data[4]);

                float x = *reinterpret_cast<const float *>(&msg->data[5]);
                float y = *reinterpret_cast<const float *>(&msg->data[9]);

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
                case EntityType::COMPANION: {
                    auto itComp = companionEntities.find(serverEntity);
                    if (itComp != companionEntities.end()) {
                        Entity localEntity = itComp->second;
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
            _audioManager.playSound(_config.getAudioConfig().shoot_sound);
            if (msg->data.size() >= 34) {
                Entity serverProjectileEntity = (static_cast<Entity>(msg->data[0]) << 24) |
                                                (static_cast<Entity>(msg->data[1]) << 16) |
                                                (static_cast<Entity>(msg->data[2]) << 8) |
                                                static_cast<Entity>(msg->data[3]);

                Entity serverParentEntity = (static_cast<Entity>(msg->data[4]) << 24) |
                                            (static_cast<Entity>(msg->data[5]) << 16) |
                                            (static_cast<Entity>(msg->data[6]) << 8) |
                                            static_cast<Entity>(msg->data[7]);

                std::string ownerType;
                for (size_t i = 8; i < 18; ++i) {
                    ownerType += static_cast<char>(msg->data[i]);
                }
                ownerType.erase(std::find(ownerType.begin(), ownerType.end(), '\0'),
                                ownerType.end());

                // Decode x coordinate
                uint32_t xInt = (static_cast<uint32_t>(msg->data[18]) << 24) |
                                (static_cast<uint32_t>(msg->data[19]) << 16) |
                                (static_cast<uint32_t>(msg->data[20]) << 8) |
                                static_cast<uint32_t>(msg->data[21]);
                float x;
                std::memcpy(&x, &xInt, sizeof(float));

                // Decode y coordinate
                uint32_t yInt = (static_cast<uint32_t>(msg->data[22]) << 24) |
                                (static_cast<uint32_t>(msg->data[23]) << 16) |
                                (static_cast<uint32_t>(msg->data[24]) << 8) |
                                static_cast<uint32_t>(msg->data[25]);
                float y;
                std::memcpy(&y, &yInt, sizeof(float));

                // Decode vx
                uint32_t vxInt = (static_cast<uint32_t>(msg->data[26]) << 24) |
                                (static_cast<uint32_t>(msg->data[27]) << 16) |
                                (static_cast<uint32_t>(msg->data[28]) << 8) |
                                static_cast<uint32_t>(msg->data[29]);
                float vx;
                std::memcpy(&vx, &vxInt, sizeof(float));

                // Decode vy
                uint32_t vyInt = (static_cast<uint32_t>(msg->data[30]) << 24) |
                                (static_cast<uint32_t>(msg->data[31]) << 16) |
                                (static_cast<uint32_t>(msg->data[32]) << 8) |
                                static_cast<uint32_t>(msg->data[33]);
                float vy;
                std::memcpy(&vy, &vyInt, sizeof(float));

                float scale = 1.0f;
                if (msg->data.size() >= 38) {
                    uint32_t scaleInt = (static_cast<uint32_t>(msg->data[34]) << 24) |
                                        (static_cast<uint32_t>(msg->data[35]) << 16) |
                                        (static_cast<uint32_t>(msg->data[36]) << 8) |
                                        static_cast<uint32_t>(msg->data[37]);
                    std::memcpy(&scale, &scaleInt, sizeof(float));
                }

                auto itOldProj = projectileEntities.find(serverProjectileEntity);
                if (itOldProj != projectileEntities.end()) {
                    _reg.destroyEntity(itOldProj->second);
                    projectileEntities.erase(itOldProj);
                }

                auto itOldEnemy = enemyEntities.find(serverProjectileEntity);
                if (itOldEnemy != enemyEntities.end()) {
                    std::cout << "[INFO] Cleaning up stale ENEMY (ID: " << serverProjectileEntity
                              << ") for new PROJECTILE" << std::endl;
                    _reg.destroyEntity(itOldEnemy->second);
                    enemyEntities.erase(itOldEnemy);
                }

                int size = static_cast<int>(16 * scale);

                if (ownerType == "player") {
                    auto it = playerEntities.find(serverParentEntity);
                    auto itComp = companionEntities.find(serverParentEntity);

                    if (it != playerEntities.end()) {
                        Entity localParent = it->second;

                        // Sync cooldown with server confirmation
                        _weaponsys.resetCooldown(_reg, localParent);

                        Entity projectile = _reg.createEntity();
                        _reg.addComponent<Position>(projectile, x, y);
                        _reg.addComponent<Velocity>(projectile, vx, vy);
                        _reg.addComponent<SpriteSheets>(projectile, std::string(""),
                                                        std::string("projectile_player"), size,
                                                        size, 0, 4, 0, 0.f, 0.f, true, true);

                        projectileEntities[serverProjectileEntity] = projectile;
                        std::cout << "Created player projectile (serverId: "
                                  << serverProjectileEntity << ", localId: " << projectile
                                  << ") at (" << x << ", " << y << ") scale: " << scale
                                  << std::endl;
                    } else if (itComp != companionEntities.end()) {
                        Entity projectile = _reg.createEntity();
                        _reg.addComponent<Position>(projectile, x, y);
                        _reg.addComponent<Velocity>(projectile, vx, vy);
                        _reg.addComponent<SpriteSheets>(projectile, std::string(""),
                                                        std::string("projectile_player"), size,
                                                        size, 0, 4, 0, 0.f, 0.f, true, true);

                        projectileEntities[serverProjectileEntity] = projectile;
                        std::cout << "Created companion projectile (serverId: "
                                  << serverProjectileEntity << ", localId: " << projectile
                                  << ") at (" << x << ", " << y << ") scale: " << scale
                                  << std::endl;
                    } else {
                        std::cout << "Received SHOOT from unknown entity: " << serverParentEntity
                                  << std::endl;
                    }
                } else if (ownerType == "enemy") {
                    auto it = enemyEntities.find(serverParentEntity);
                    if (it != enemyEntities.end()) {
                        Entity projectile = _reg.createEntity();
                        _reg.addComponent<Position>(projectile, x, y);
                        _reg.addComponent<Velocity>(projectile, 0.f, 200.f);
                        _reg.addComponent<SpriteSheets>(projectile, std::string(""),
                                                        std::string("projectile_player"), size,
                                                        size, 0, 4, 0, 0.f, 0.f, true, true);

                        projectileEntities[serverProjectileEntity] = projectile;
                        std::cout << "Created enemy projectile (serverId: "
                                  << serverProjectileEntity << ", localId: " << projectile
                                  << ") at (" << x << ", " << y << ") scale: " << scale
                                  << std::endl;
                    }
                } else if (ownerType == "boss_orb") {
                    Entity projectile = _reg.createEntity();
                    _reg.addComponent<Position>(projectile, x, y);
                    _reg.addComponent<Velocity>(projectile, 0.f, 200.f);

                    _renderer.loadTexture("textures/projectiles/boss_orb.png", "boss_orb");
                    _reg.addComponent<Sprite>(
                        projectile, std::string("textures/projectiles/boss_orb.png"),
                        std::string("boss_orb"), size, size, 10, 0.f, 0.f, true);

                    projectileEntities[serverProjectileEntity] = projectile;

                } else {
                    std::cout << "Invalid ownerType in SHOOT message: " << ownerType << std::endl;
                }
            }
            break;
        }
        case OpCode::ENEMY: {
            if (msg->data.size() >= 28) {
                Entity serverEntity = (static_cast<Entity>(msg->data[0]) << 24) |
                                      (static_cast<Entity>(msg->data[1]) << 16) |
                                      (static_cast<Entity>(msg->data[2]) << 8) |
                                      static_cast<Entity>(msg->data[3]);

                float x = *reinterpret_cast<const float *>(&msg->data[4]);
                float y = *reinterpret_cast<const float *>(&msg->data[8]);

                char typeBuf[17];
                std::memcpy(typeBuf, &msg->data[12], 16);
                typeBuf[16] = '\0';
                std::string typeStr = typeBuf;

                // Clean nulls if any
                size_t firstNull = typeStr.find('\0');
                if (firstNull != std::string::npos) {
                    typeStr = typeStr.substr(0, firstNull);
                }

                auto it = enemyEntities.find(serverEntity);
                if (it == enemyEntities.end()) {
                    // Check if this ID is already used by another type (stale packet
                    // check)
                    if (companionEntities.find(serverEntity) != companionEntities.end()) {
                        std::cout << "[INFO] Ignoring stale ENEMY packet for existing COMPANION "
                                  << serverEntity << std::endl;
                        break;
                    }
                    if (playerEntities.find(serverEntity) != playerEntities.end()) {
                        std::cout << "[INFO] Ignoring stale ENEMY packet for existing PLAYER "
                                  << serverEntity << std::endl;
                        break;
                    }

                    // Force clean up stale projectiles if they exist (Fixes ID collision glitches)
                    auto itProj = projectileEntities.find(serverEntity);
                    if (itProj != projectileEntities.end()) {
                        std::cout << "[INFO] Cleaning up stale PROJECTILE (ID: " << serverEntity 
                                  << ") for new ENEMY" << std::endl;
                        _reg.destroyEntity(itProj->second);
                        projectileEntities.erase(itProj);
                    }

                    cleanupServerEntity(serverEntity);

                    Entity localEntity = _reg.createEntity();
                    _reg.addComponent<Position>(localEntity, x, y);
                    _reg.addComponent<Velocity>(localEntity, 0.f, 0.f);

                    // Check for Boss Config
                    bool loadedFromConfig = false;
                    const auto &bosses = _config.getBosses();
                    for (const auto &[id, config] : bosses) {
                        if (config.name == typeStr) {
                            int w = (int)config.visuals.width;
                            int h = (int)config.visuals.height;
                            int frames = 1;

                            if (config.visuals.animations.count("idle")) {
                                const auto &anim = config.visuals.animations.at("idle");
                                w = anim.width;
                                h = anim.height;
                                frames = anim.frame_count;
                            }

                            _reg.addComponent<SpriteSheets>(
                                localEntity, config.visuals.texture_path, config.name, w, h, 0,
                                frames, 4, config.visuals.offset_x, config.visuals.offset_y, true,
                                true);
                            std::cout << "Created BOSS entity (serverId: " << serverEntity
                                      << ") via Config: " << config.name << " Frames: " << frames
                                      << std::endl;
                            loadedFromConfig = true;
                            break;
                        }
                    }

                    if (!loadedFromConfig) {
                        if (typeStr == "Ancient Obelisk") {
                            _reg.addComponent<SpriteSheets>(
                                localEntity, std::string("textures/ships/bosses/Obelisk.png"),
                                std::string("Ancient Obelisk"), 240, 160, 0, 14, 4, 0.f, 0.f, true,
                                true);
                            std::cout << "Created BOSS entity (serverId: " << serverEntity
                                      << ") type: " << typeStr << " (Fallback)" << std::endl;
                        } else if (typeStr == "void_zone") {
                            _renderer.loadTexture("textures/ships/bosses/Obelisk_effects.png",
                                                  "obelisk_void");
                            _reg.addComponent<Sprite>(
                                localEntity,
                                std::string("textures/ships/bosses/Obelisk_effects.png"),
                                std::string("obelisk_void"), 100, 100, 5, -50.f, -50.f, true);
                            std::cout << "Created VOID ZONE entity (serverId: " << serverEntity
                                      << ")" << std::endl;
                        } else {
                            _reg.addComponent<Sprite>(
                                localEntity, std::string("textures/ships/enemy_ship.png"),
                                std::string("enemy_ship"), 50, 50, 0, 0.f, 0.f, true);
                            std::cout << "Created enemy entity (serverId: " << serverEntity
                                      << ") type: " << typeStr << std::endl;
                        }
                    }

                    enemyEntities[serverEntity] = localEntity;
                } else {
                    Entity localEntity = it->second;
                    _reg.getComponent<Position>(localEntity).x = x;
                    _reg.getComponent<Position>(localEntity).y = y;
                }
            }
            break;
        }
        case OpCode::COMPANION: {
            if (msg->data.size() >= 13) {
                Entity serverEntity = (static_cast<Entity>(msg->data[0]) << 24) |
                                      (static_cast<Entity>(msg->data[1]) << 16) |
                                      (static_cast<Entity>(msg->data[2]) << 8) |
                                      static_cast<Entity>(msg->data[3]);

                float x, y;
                std::memcpy(&x, &msg->data[4], sizeof(float));
                std::memcpy(&y, &msg->data[8], sizeof(float));

                std::cout << "[DEBUG] COMPANION OpCode Received: ID=" << serverEntity << " X=" << x
                          << " Y=" << y << std::endl;

                auto it = companionEntities.find(serverEntity);
                if (it == companionEntities.end()) {
                    cleanupServerEntity(serverEntity);

                    Entity localEntity = _reg.createEntity();
                    _reg.addComponent<Position>(localEntity, x, y);
                    _reg.addComponent<Velocity>(localEntity, 0.f, 0.f);

                    // Use player_ship texture for drone (since vaisseau.png is missing)
                    // We scale it down to 30x30 to look like a small drone
                    _reg.addComponent<Sprite>(localEntity,
                                              std::string("textures/ships/player_ship.png"),
                                              std::string("drone"), 30, 30, 15, 0.f, 0.f, true);
                    std::cout << "Created COMPANION entity (serverId: " << serverEntity << ")"
                              << std::endl;

                    companionEntities[serverEntity] = localEntity;
                } else {
                    // Update position if already exists
                    Entity localEntity = it->second;
                    _reg.getComponent<Position>(localEntity).x = x;
                    _reg.getComponent<Position>(localEntity).y = y;
                }
            } else {
                std::cerr << "[ERROR] COMPANION packet size too small: " << msg->data.size()
                          << std::endl;
            }
            break;
        }
        case OpCode::UPGRADE_OPTIONS: {
            handleUpgradeOptions(*msg);
            break;
        }
        case OpCode::UPDATE_WEAPON: {
            handleUpdateWeapon(*msg);
            break;
        }
        case OpCode::UPDATE_STATS: {
            if (msg->data.size() >= 16) {
                Entity serverEntity = (static_cast<Entity>(msg->data[0]) << 24) |
                                      (static_cast<Entity>(msg->data[1]) << 16) |
                                      (static_cast<Entity>(msg->data[2]) << 8) |
                                      static_cast<Entity>(msg->data[3]);

                int hp = (static_cast<int>(msg->data[4]) << 24) |
                         (static_cast<int>(msg->data[5]) << 16) |
                         (static_cast<int>(msg->data[6]) << 8) | static_cast<int>(msg->data[7]);

                int maxHp = (static_cast<int>(msg->data[8]) << 24) |
                            (static_cast<int>(msg->data[9]) << 16) |
                            (static_cast<int>(msg->data[10]) << 8) |
                            static_cast<int>(msg->data[11]);

                int speed = (static_cast<int>(msg->data[12]) << 24) |
                            (static_cast<int>(msg->data[13]) << 16) |
                            (static_cast<int>(msg->data[14]) << 8) |
                            static_cast<int>(msg->data[15]);

                auto it = playerEntities.find(serverEntity);
                if (it != playerEntities.end()) {
                    Entity localEntity = it->second;
                    if (_reg.hasComponent<Stats>(localEntity)) {
                        auto &stats = _reg.getComponent<Stats>(localEntity);
                        stats.hp = hp;
                        stats.maxHp = maxHp;
                        stats.movement_speed = speed;
                        std::cout << "Updated Stats for player " << serverEntity << ": HP=" << hp
                                  << "/" << maxHp << " Speed=" << speed << std::endl;
                    }
                }
            }
            break;
        }
        case OpCode::DEATH: {
            if (msg->data.size() >= 5) {
                EntityType type = static_cast<EntityType>(msg->data[0]);

                Entity serverEntity = (static_cast<Entity>(msg->data[1]) << 24) |
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
                case EntityType::COMPANION: {
                    auto it = companionEntities.find(serverEntity);
                    if (it != companionEntities.end()) {
                        _reg.destroyEntity(it->second);
                        companionEntities.erase(it);
                        std::cout << "Companion " << serverEntity << " destroyed" << std::endl;
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

void ClientGameHandler::handlePlayerPacket(const DecodedMessage &msg) {
    if (msg.data.size() < 17) {
        return;
    }
    if (myPlayerId == 0) {
        pendingPlayerPackets.push_back(msg);
        return;
    }

    uint32_t playerId =
        (static_cast<uint32_t>(msg.data[0]) << 24) | (static_cast<uint32_t>(msg.data[1]) << 16) |
        (static_cast<uint32_t>(msg.data[2]) << 8) | static_cast<uint32_t>(msg.data[3]);

    Entity serverEntity =
        (static_cast<Entity>(msg.data[4]) << 24) | (static_cast<Entity>(msg.data[5]) << 16) |
        (static_cast<Entity>(msg.data[6]) << 8) | static_cast<Entity>(msg.data[7]);

    float x = *reinterpret_cast<const float *>(&msg.data[8]);
    float y = *reinterpret_cast<const float *>(&msg.data[12]);
    
    uint8_t skinIndex = msg.data[16];

    auto it = playerEntities.find(serverEntity);
    static std::vector<std::string> shipSkins = {"player_ship", "player_ship_blue",
                                                 "player_ship_green", "player_ship_yellow"};
    if (it == playerEntities.end()) {
        // Nettoyer les anciennes références AVANT de créer la nouvelle entité
        cleanupServerEntity(serverEntity);

        std::string selectedSkin = shipSkins[skinIndex % shipSkins.size()];

        Entity localEntity = _reg.createEntity();
        _reg.addComponent<Position>(localEntity, x, y);
        _reg.addComponent<Velocity>(localEntity, 0.f, 0.f);
        // Use config for sprite dimensions
        int sprW = (int)_config.getPlayerConfig().hitbox.sprite_width;
        int sprH = (int)_config.getPlayerConfig().hitbox.sprite_height;
        auto &pStats = _config.getPlayerConfig().stats;

        _reg.addComponent<SpriteSheets>(localEntity, std::string(""), selectedSkin, sprW, sprH, 1,
                                        3, 0, 0.f, 0.f, true, false);
        _reg.addComponent<Stats>(localEntity, pStats.health, pStats.max_health, pStats.attack_speed,
                                 0.f, pStats.attack_damage, 1, pStats.speed);
        _reg.addComponent<Weapon>(localEntity, pStats.attack_damage, 1, 0.5f);
        _weaponsys.setWeaponType(_reg, localEntity, WeaponType::DEFAULT);

        // Override with config stats
        if (_reg.hasComponent<Weapon>(localEntity)) {
            auto &w = _reg.getComponent<Weapon>(localEntity);
            w.damage = pStats.attack_damage;
            if (pStats.attack_speed > 0) {
                w.fireRate = 5.0f / (float)pStats.attack_speed;
            }
        }

        playerEntities[serverEntity] = localEntity;

        if (playerId == myPlayerId) {
            myEntity = localEntity;
            _input.setControlled(localEntity, _keybindsManager);
            std::cout << "Created my player entity (serverId: " << serverEntity
                      << ", localId: " << localEntity << ") at (" << x << ", " << y << ") skin: " << selectedSkin
                      << std::endl;
        } else {
            std::cout << "Created other player entity (serverId: " << serverEntity
                      << ", localId: " << localEntity << ") at (" << x << ", " << y << ") skin: " << selectedSkin
                      << std::endl;
        }
    } else {
        Entity localEntity = it->second;
        _reg.getComponent<Position>(localEntity).x = x;
        _reg.getComponent<Position>(localEntity).y = y;
    }
}

void ClientGameHandler::toggleSettingsMenu() {
    settingsMenuOpen = !settingsMenuOpen;
    _settingsMenu.toggle(_reg);
}

void ClientGameHandler::handleUpgradeOptions(const DecodedMessage &msg) {
    // Parse message
    if (msg.data.empty())
        return;

    // Format: [Count] [Len1][Str1] [Len2][Str2] ...
    uint8_t count = msg.data[0];

    std::vector<std::string> ids;
    size_t offset = 1;

    for (int i = 0; i < count; ++i) {
        if (offset >= msg.data.size())
            break;

        uint8_t len = msg.data[offset];
        offset++;

        if (offset + len > msg.data.size())
            break;

        std::string id(msg.data.begin() + offset, msg.data.begin() + offset + len);
        ids.push_back(id);

        offset += len;
    }

    std::cout << "Received UPGRADE_OPTIONS (" << ids.size() << ")" << std::endl;
    showUpgradeMenu(ids);
}

void ClientGameHandler::showUpgradeMenu(const std::vector<std::string> &ids) {
    if (upgradeMenuOpen)
        return;
    upgradeMenuOpen = true;

    // Position helpers
    float startX = 200.0f;
    float startY = 300.0f;
    float gap = 250.0f;

    // Find upgrade data
    const std::vector<UpgradeData> &allUpgrades = _config.getUpgrades();

    for (size_t i = 0; i < ids.size(); ++i) {
        std::string id = ids[i];

        // Find data
        auto it = std::find_if(allUpgrades.begin(), allUpgrades.end(),
                               [&](const UpgradeData &u) { return u.id == id; });

        if (it != allUpgrades.end()) {
            const UpgradeData &data = *it;

            float x = startX + (i * gap);

            // Create UI Entities
            // 1. Background/Button (Invisible clickable area or Texture)
            Entity btn = _reg.createEntity();
            _reg.addComponent<Position>(btn, x, startY);
            // Upgrade card background - Z Index 20
            _reg.addComponent<Sprite>(btn, (std::string) "textures/upgrades/border.png",
                                      (std::string) "upgrade_border", 220, 300, 20, 0.f, 0.f, true);

            std::string btnId = "upgrade_" + std::to_string(i);
            _reg.addComponent<Button>(btn, btnId, 1, true);

            upgradeMenuEntities.push_back(btn);

            // 2. Name
            Entity nameLbl = _reg.createEntity();
            // Padding added (x+35, y+40)
            _reg.addComponent<Position>(nameLbl, x + 35, startY + 40);
            _reg.addComponent<Label>(
                nameLbl, data.name, (std::string) "font/josefin-sans/JosefinSans-Regular.ttf",
                (std::string) "default_font_medium", Color(255, 255, 255), 21, true);
            upgradeMenuEntities.push_back(nameLbl);

            // 3. Description (Multi-line word wrap)
            std::istringstream iss(data.description);
            std::string word;
            std::string currentLine;
            // Approx chars per line for font size 15 in width ~150px
            size_t maxLineChars = 15;
            std::vector<std::string> lines;

            while (iss >> word) {
                if (currentLine.length() + word.length() + 1 > maxLineChars) {
                    lines.push_back(currentLine);
                    currentLine = word;
                } else {
                    if (!currentLine.empty())
                        currentLine += " ";
                    currentLine += word;
                }
            }
            if (!currentLine.empty())
                lines.push_back(currentLine);

            float descY = startY + 80;
            for (const auto &line : lines) {
                Entity lineLbl = _reg.createEntity();
                _reg.addComponent<Position>(lineLbl, x + 35, descY);
                _reg.addComponent<Label>(
                    lineLbl, line, (std::string) "font/josefin-sans/JosefinSans-Regular.ttf",
                    (std::string) "default_font_tiny", Color(200, 200, 200), 21, true);
                upgradeMenuEntities.push_back(lineLbl);
                descY += 20;
            }

            // 4. Rarity
            Color rarityColor(255, 255, 255);
            std::string rLower = data.rarity;

            if (rLower == "common")
                rarityColor = Color(200, 200, 200); // Gray
            else if (rLower == "rare")
                rarityColor = Color(30, 144, 255); // Blue
            else if (rLower == "epic")
                rarityColor = Color(186, 85, 211); // Purple
            else if (rLower == "legendary")
                rarityColor = Color(255, 215, 0); // Gold

            // Uppercase first letter
            std::string displayRarity = rLower;
            if (!displayRarity.empty())
                displayRarity[0] = std::toupper(displayRarity[0]);

            Entity rarityLbl = _reg.createEntity();
            // Bottom (300 height) - 60 padding = 240
            _reg.addComponent<Position>(rarityLbl, x + 35, startY + 240);
            _reg.addComponent<Label>(rarityLbl, displayRarity,
                                     (std::string) "font/josefin-sans/JosefinSans-Regular.ttf",
                                     (std::string) "default_font_tiny", rarityColor, 21, true);
            upgradeMenuEntities.push_back(rarityLbl);

            _buttonsys.registerHandler(btnId, [this, i](Registry &r, Entity e) {
                (void)r;
                (void)e;
                this->selectUpgrade(i);
            });
        } else {
            std::cout << "Upgrade ID not found in config: " << id << std::endl;
        }
    }
}

void ClientGameHandler::selectUpgrade(int index) {
    if (!upgradeMenuOpen)
        return;

    std::cout << "Selected Upgrade Index: " << index << std::endl;

    // Cleanup UI
    for (auto e : upgradeMenuEntities) {
        _reg.destroyEntity(e);
    }
    upgradeMenuEntities.clear();
    upgradeMenuOpen = false;

    // Send choice to server
    MessageFactory &factory = MessageFactory::getInstance();
    PreparedMessage msg =
        factory.createMessage(OpCode::UPGRADE_SELECT, factory.encodeMessageUpgradeSelect(index));
    _network.sendUdp(msg);
}

void ClientGameHandler::handleUpdateWeapon(const DecodedMessage &msg) {
    if (msg.data.size() < 16)
        return;

    // Entity (4)
    Entity serverEntity =
        (static_cast<Entity>(msg.data[0]) << 24) | (static_cast<Entity>(msg.data[1]) << 16) |
        (static_cast<Entity>(msg.data[2]) << 8) | static_cast<Entity>(msg.data[3]);

    // Damage (4)
    int damage = (static_cast<int>(msg.data[4]) << 24) | (static_cast<int>(msg.data[5]) << 16) |
                 (static_cast<int>(msg.data[6]) << 8) | static_cast<int>(msg.data[7]);

    // NbBullets (4)
    int nbBullets = (static_cast<int>(msg.data[8]) << 24) | (static_cast<int>(msg.data[9]) << 16) |
                    (static_cast<int>(msg.data[10]) << 8) | static_cast<int>(msg.data[11]);

    // FireRate (4)
    float fireRate = *reinterpret_cast<const float *>(&msg.data[12]);

    // Find local entity (Player)
    auto itPlayer = playerEntities.find(serverEntity);
    if (itPlayer != playerEntities.end()) {
        Entity localEntity = itPlayer->second;
        if (_reg.hasComponent<Weapon>(localEntity)) {
            auto &w = _reg.getComponent<Weapon>(localEntity);
            w.damage = damage;
            w.nbOfBullets = nbBullets;
            w.fireRate = fireRate;
            std::cout << "Updated Weapon for player " << serverEntity << " (local " << localEntity
                      << "): FR=" << fireRate << " DMG=" << damage << std::endl;
        } else {
            _reg.addComponent<Weapon>(localEntity, damage, nbBullets, fireRate);
            std::cout << "Added Weapon for player " << serverEntity << " (local " << localEntity
                      << ")" << std::endl;
        }
    }
}

void ClientGameHandler::setupLobbyCallbacks() {
    _lobbyMenu.setJoinCallback([this](uint32_t roomId) { joinRoom(roomId); });

    _lobbyMenu.setCreateCallback([this]() { showCreateRoomMenu(); });

    _lobbyMenu.setRefreshCallback([this]() { requestRoomList(); });

    // CreateRoomMenu callbacks
    _createRoomMenu.setOnConfirm([this](const RoomConfig& config) {
        createRoom(config);
        _createRoomMenu.hide();
        _lobbyMenu.show();
        _gameState = GameState::LOBBY;
    });

    _createRoomMenu.setOnCancel([this]() {
        _createRoomMenu.hide();
        _lobbyMenu.show();
        _gameState = GameState::LOBBY;
    });
}

void ClientGameHandler::requestRoomList() {
    MessageFactory &factory = MessageFactory::getInstance();
    PreparedMessage listMsg = factory.createMessage(OpCode::LIST_ROOMS, {});
    _network.sendTcp(listMsg);
    std::cout << "Requesting room list..." << std::endl;
}

void ClientGameHandler::showCreateRoomMenu() {
    std::cout << "Opening room creation menu..." << std::endl;
    _lobbyMenu.hide();
    _createRoomMenu.show();
    _gameState = GameState::CREATE_ROOM;
}

void ClientGameHandler::createRoom(const RoomConfig& config) {
    MessageFactory &factory = MessageFactory::getInstance();
    MessageData payload = factory.encodeMessageCreateRoom(
        config.maxPlayers,
        static_cast<uint8_t>(config.gameMode),
        static_cast<uint8_t>(config.difficulty)
    );
    PreparedMessage createMsg = factory.createMessage(OpCode::CREATE_ROOM, payload);
    _network.sendTcp(createMsg);
    std::cout << "Creating new room (MaxPlayers: " << (int)config.maxPlayers 
              << ", Mode: " << config.getGameModeStr() 
              << ", Difficulty: " << config.getDifficultyStr() << ")..." << std::endl;
}

void ClientGameHandler::joinRoom(uint32_t roomId) {
    MessageFactory &factory = MessageFactory::getInstance();
    std::vector<uint8_t> payload;
    payload.push_back(static_cast<uint8_t>((roomId >> 24) & 0xFF));
    payload.push_back(static_cast<uint8_t>((roomId >> 16) & 0xFF));
    payload.push_back(static_cast<uint8_t>((roomId >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(roomId & 0xFF));

    PreparedMessage joinMsg = factory.createMessage(OpCode::JOIN_ROOM, payload);
    _network.sendTcp(joinMsg);
    std::cout << "Joining room " << roomId << "..." << std::endl;
}

void ClientGameHandler::registerJoinHandler(uint32_t roomId) {
    std::string handlerName = "lobby_join_" + std::to_string(roomId);
    std::cout << "[DEBUG] Registering handler: " << handlerName << std::endl;
    _buttonsys.registerHandler(handlerName, [this, roomId](Registry &, Entity) {
        std::cout << "[DEBUG] Join button clicked for room " << roomId << std::endl;
        if (_lobbyMenu.isVisible()) {
            joinRoom(roomId);
        }
    });
}
