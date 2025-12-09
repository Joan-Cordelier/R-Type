/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Client main entry point
*/

#include "graphic/Renderer.hpp"
#include "../common/ecs/registry.hpp"
#include "../common/ecs/systems/movement_system.hpp"
#include "../common/ecs/systems/sprite_system.hpp"
#include "../common/ecs/systems/button_system.hpp"
#include "../common/ecs/systems/label_system.hpp"
#include "../common/ecs/systems/spritesheet_system.hpp"
#include "../common/ecs/systems/stat_system.hpp"
#include "input_system.hpp"

#include "../common/ecs/components/position.hpp"
#include "../common/ecs/components/velocity.hpp"
#include "../common/ecs/components/sprite.hpp"
#include "../common/ecs/components/button.hpp"
#include "../common/ecs/components/label.hpp"
#include "../common/ecs/components/stats.hpp"
#include "../common/ecs/components/spritesheet.hpp"

#include "Network/NetworkManager.hpp"
#include "../common/Data/MessageFactory.hpp"

#include <functional>
#include <map>
#include <vector>
#include <SDL2/SDL.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void handleMessages(NetworkManager& network, Registry& reg, Renderer& renderer, 
                    std::map<Entity, Entity>& playerEntities, uint32_t& myPlayerId,
                    Entity& myEntity, InputSystem& input,
                    std::vector<DecodedMessage>& pendingPlayerPackets)
{
    std::optional<DecodedMessage> msg;
    while (network.hasMessages()) {
        msg = network.popMessage(Priority::CRITICAL);
        if (msg.has_value() == false) {
            msg = network.popMessage(Priority::HIGH);
            if (msg.has_value() == false) {
                msg = network.popMessage(Priority::MEDIUM);
                if (msg.has_value() == false) {
                    msg = network.popMessage(Priority::LOW);
                    if (msg.has_value() == false) {
                        msg = network.popMessage(Priority::ERROR);
                        if (msg.has_value() == false) {
                            break;
                        }
                    }
                }
            }
        }

        switch (msg->opCode) {
            case OpCode::CONNECT_ACK: {
                // Server responds to CONNECT with CONNECT_ACK containing playerId
                if (msg->data.size() >= 4) {
                    myPlayerId = 
                        (static_cast<uint32_t>(msg->data[0]) << 24) |
                        (static_cast<uint32_t>(msg->data[1]) << 16) |
                        (static_cast<uint32_t>(msg->data[2]) << 8) |
                        static_cast<uint32_t>(msg->data[3]);
                    std::cout << "Received playerId: " << myPlayerId << std::endl;

                    // Send LINK message via UDP to associate our UDP address with our playerId
                    MessageFactory& factory = MessageFactory::getInstance();
                    std::vector<uint8_t> linkPayload;
                    linkPayload.push_back(static_cast<uint8_t>((myPlayerId >> 24) & 0xFF));
                    linkPayload.push_back(static_cast<uint8_t>((myPlayerId >> 16) & 0xFF));
                    linkPayload.push_back(static_cast<uint8_t>((myPlayerId >> 8) & 0xFF));
                    linkPayload.push_back(static_cast<uint8_t>(myPlayerId & 0xFF));
                    PreparedMessage linkMsg = factory.createMessage(OpCode::LINK, linkPayload);
                    network.sendUdp(linkMsg);
                    
                    // Get our local UDP port using getsockname
                    struct sockaddr_in localAddr;
                    socklen_t addrLen = sizeof(localAddr);
                    int udpFd = network.getUdpClient().getSocketFd();
                    if (getsockname(udpFd, (struct sockaddr*)&localAddr, &addrLen) == 0) {
                        uint16_t localPort = ntohs(localAddr.sin_port);
                        std::cout << "Sent LINK message via UDP, listening on port: " << localPort << std::endl;
                    } else {
                        std::cout << "Sent LINK message via UDP" << std::endl;
                    }
                    
                    // Process any PLAYER packets that arrived before CONNECT_ACK
                    for (auto& pendingMsg : pendingPlayerPackets) {
                        if (pendingMsg.data.size() >= 16) {
                            uint32_t pendingPlayerId = 
                                (static_cast<uint32_t>(pendingMsg.data[0]) << 24) |
                                (static_cast<uint32_t>(pendingMsg.data[1]) << 16) |
                                (static_cast<uint32_t>(pendingMsg.data[2]) << 8) |
                                static_cast<uint32_t>(pendingMsg.data[3]);
                            
                            Entity serverEntity = 
                                (static_cast<Entity>(pendingMsg.data[4]) << 24) |
                                (static_cast<Entity>(pendingMsg.data[5]) << 16) |
                                (static_cast<Entity>(pendingMsg.data[6]) << 8) |
                                static_cast<Entity>(pendingMsg.data[7]);
                            
                            float x = *reinterpret_cast<const float*>(&pendingMsg.data[8]);
                            float y = *reinterpret_cast<const float*>(&pendingMsg.data[12]);
                            
                            auto it = playerEntities.find(serverEntity);
                            if (it == playerEntities.end()) {
                                Entity localEntity = reg.createEntity();
                                reg.addComponent<Position>(localEntity, x, y);
                                reg.addComponent<Velocity>(localEntity, 0.f, 0.f);
                                reg.addComponent<SpriteSheets>(localEntity, std::string("textures/ships/player_ship.png"), std::string("test"), 120, 130, 1, 3, 0, false, true);
                                reg.addComponent<Stats>(localEntity, 100, 100, 1, 0.f, 10, 1, 200);
                                
                                playerEntities[serverEntity] = localEntity;
                                
                                if (pendingPlayerId == myPlayerId) {
                                    myEntity = localEntity;
                                    input.setControlled(localEntity);
                                    std::cout << "Created my player entity from pending (serverId: " << serverEntity << ", localId: " << localEntity << ")" << std::endl;
                                } else {
                                    std::cout << "Created other player entity from pending (serverId: " << serverEntity << ", localId: " << localEntity << ")" << std::endl;
                                }
                            }
                        }
                    }
                    pendingPlayerPackets.clear();
                }
                break;
            }
            case OpCode::PLAYER: {
                // PLAYER packet: playerId (4) + entityId (4) + x (4) + y (4)
                if (msg->data.size() >= 16) {
                    // If we don't have our playerId yet, queue this packet
                    if (myPlayerId == 0) {
                        pendingPlayerPackets.push_back(*msg);
                        std::cout << "Queued PLAYER packet (waiting for CONNECT_ACK)" << std::endl;
                        break;
                    }
                    
                    uint32_t playerId = 
                        (static_cast<uint32_t>(msg->data[0]) << 24) |
                        (static_cast<uint32_t>(msg->data[1]) << 16) |
                        (static_cast<uint32_t>(msg->data[2]) << 8) |
                        static_cast<uint32_t>(msg->data[3]);
                    
                    Entity serverEntity = 
                        (static_cast<Entity>(msg->data[4]) << 24) |
                        (static_cast<Entity>(msg->data[5]) << 16) |
                        (static_cast<Entity>(msg->data[6]) << 8) |
                        static_cast<Entity>(msg->data[7]);
                    
                    float x = *reinterpret_cast<const float*>(&msg->data[8]);
                    float y = *reinterpret_cast<const float*>(&msg->data[12]);
                    
                    // Check if we already have this player
                    auto it = playerEntities.find(serverEntity);
                    if (it == playerEntities.end()) {
                        // Create new player entity
                        Entity localEntity = reg.createEntity();
                        reg.addComponent<Position>(localEntity, x, y);
                        reg.addComponent<Velocity>(localEntity, 0.f, 0.f);
                        reg.addComponent<SpriteSheets>(localEntity, std::string("textures/ships/player_ship.png"), std::string("test"), 120, 130, 1, 3, 0, true, true);
                        reg.addComponent<Stats>(localEntity, 100, 100, 1, 0.f, 10, 1, 200);
                        
                        playerEntities[serverEntity] = localEntity;
                        
                        // If this is our own player, set it as controlled
                        std::cout << "Player ID in PLAYER packet: " << playerId << ", myPlayerId: " << myPlayerId << std::endl;
                        if (playerId == myPlayerId) {
                            myEntity = localEntity;
                            input.setControlled(localEntity);
                            std::cout << "Created my player entity (serverId: " << serverEntity << ", localId: " << localEntity << ") at (" << x << ", " << y << ")" << std::endl;
                        } else {
                            std::cout << "Created other player entity (serverId: " << serverEntity << ", localId: " << localEntity << ") at (" << x << ", " << y << ")" << std::endl;
                        }
                    } else {
                        // Update existing player position
                        Entity localEntity = it->second;
                        reg.getComponent<Position>(localEntity).x = x;
                        reg.getComponent<Position>(localEntity).y = y;
                    }
                }
                break;
            }
            case OpCode::MOVE: {
                // MOVE packet: entityId (4) + x (4) + y (4)
                if (msg->data.size() >= 12) {
                    Entity serverEntity = 
                        (static_cast<Entity>(msg->data[0]) << 24) |
                        (static_cast<Entity>(msg->data[1]) << 16) |
                        (static_cast<Entity>(msg->data[2]) << 8) |
                        static_cast<Entity>(msg->data[3]);
                    
                    float x = *reinterpret_cast<const float*>(&msg->data[4]);
                    float y = *reinterpret_cast<const float*>(&msg->data[8]);
                    
                    auto it = playerEntities.find(serverEntity);
                    if (it != playerEntities.end()) {
                        Entity localEntity = it->second;
                        reg.getComponent<Position>(localEntity).x = x;
                        reg.getComponent<Position>(localEntity).y = y;
                    }
                }
                break;
            }
            default:
                break;
        }
    }
}

int main()
{
    Renderer renderer;
    Registry reg;
    MovementSystem movement;
    SpriteSystem spritesys;
    ButtonSystem buttonsys;
    LabelSystem labelsys;
    SpriteSheetSystem spritesheetsys;
    StatSystem statsys;
    InputSystem Input;

    NetworkManager network;
    std::string ip_adress = "127.0.0.1";
    double animationClock = 0.0;
    uint32_t myPlayerId = 0;
    Entity myEntity = 0;  // Will be set when we receive our PLAYER packet
    std::map<Entity, Entity> playerEntities;  // server entity ID -> local entity ID
    std::vector<DecodedMessage> pendingPlayerPackets;  // Queue for PLAYER packets before CONNECT_ACK

    renderer.loadSpriteSheet("textures/ships/player_ship.png", "test", 343, 383);

    renderer.loadTexture("textures/play_button/default.png", "play_button");

    Entity start_button = reg.createEntity();
    reg.addComponent<Position>(start_button, 400.f, 300.f);
    reg.addComponent<Sprite>(start_button, (std::string)"textures/play_button/default.png", (std::string)"play_button", 300, 150, 0, true);
    reg.addComponent<Button>(start_button, (std::string)"start_game", 1, true);

    renderer.loadSpriteSheet("textures/projectiles/projectile_player.png", "projectile_player", 16, 16);

    renderer.loadFont("font/josefin-sans/JosefinSans-Regular.ttf", 40, "default_font");

    Entity label_input = reg.createEntity();
    reg.addComponent<Position>(label_input, 400.f, 200.f);
    reg.addComponent<Label>(label_input, (std::string)"", (std::string)"font/josefin-sans/JosefinSans-Regular.ttf", (std::string)"default_font", Color(255, 255, 255), 0, true);

    Uint64 last = SDL_GetPerformanceCounter();
    bool running = true;

    Input.setControlled(label_input);

    buttonsys.registerHandler("start_game", [&](Registry& r, Entity e) {
        ip_adress = reg.getComponent<Label>(label_input).text;
        if (ip_adress == "") {
            ip_adress = "127.0.0.1";
        }
        if (network.connect(ip_adress, 4789, 4790) != 0) {
            std::cerr << "Failed to connect to server" << std::endl;
            return;
        }
        network.start();
        r.getComponent<Label>(label_input).visible = false;
        r.getComponent<Sprite>(e).visible = false;
        r.getComponent<Button>(e).enabled = false;

        MessageFactory& factory = MessageFactory::getInstance();
        PreparedMessage msg = factory.createMessage(OpCode::CONNECT, {});
        network.sendTcp(msg);
    });

    while (running) {
        renderer.window.processSDLEvents();
        SDL_Event status = renderer.window.pollEvent();
        if (status.type == SDL_QUIT)
            break;

        Uint64 now = SDL_GetPerformanceCounter();
        double dt = (double)(now - last) / SDL_GetPerformanceFrequency();
        animationClock += dt;
        last = now;

        handleMessages(network, reg, renderer, playerEntities, myPlayerId, myEntity, Input, pendingPlayerPackets);

        Input.update(reg, status, network);

        movement.update(reg, static_cast<float>(dt));

        buttonsys.update(reg);

        renderer.clear();

        statsys.update(reg, static_cast<float>(dt));

        spritesys.render(reg, [&](const SpriteSystem::TextureId& tid, int width, int height, int x, int y, int z) {
            renderer.drawTexture(tid, RenderLayer::GAME, z, Rect{x, y, width, height});
        });

        spritesheetsys.render(reg, [&](const SpriteSheetSystem::TextureId& tid, int frameIndex, int width, int height, int x, int y, int z) {
            renderer.drawFrame(tid, frameIndex, RenderLayer::GAME, z, Rect{x, y, width, height});
        }, animationClock);

        labelsys.render(reg, [&](const LabelSystem::TextId& tid, std::string& text, int x, int y, Color color) {
            renderer.drawFont(tid, text, x, y, color, RenderLayer::OVERLAY, 0);
        });

        renderer.render();
    }

    return 0;
}
