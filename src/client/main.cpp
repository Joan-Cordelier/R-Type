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

#include "Network/UDPClient.hpp"
#include "Network/TCPClient.hpp"
#include "../common/Data/MessageFactory.hpp"
#include "../common/Data/ThreadedQueue.hpp"

#include <functional>
#include <SDL2/SDL.h>

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

    ThreadedQueue<DecodedMessage> queue;
    std::string ip_adress = "127.0.0.1";
    UDPClient clientUDP(queue);
    TCPClient clientTCP(queue);
    double animationClock = 0.0;

    renderer.loadSpriteSheet("textures/ships/player_ship.png", "test", 343, 383);

    Entity player = reg.createEntity();
    reg.addComponent<Position>(player, 100.f, 100.f);
    reg.addComponent<Velocity>(player, 0.f, 0.f);
    reg.addComponent<SpriteSheets>(player, (std::string)"textures/ships/player_ship.png", (std::string)"test", 120, 130, 1, 3, 0, false, false);
    reg.addComponent<Stats>(player, 100, 100, 1, 0.f, 10, 1, 200);

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
        clientUDP.init(AClient::UDP, ip_adress, 4789);
        clientUDP.connect();
        clientTCP.init(AClient::TCP, ip_adress, 4789);
        clientTCP.connect();
        r.getComponent<Label>(label_input).visible = false;
        r.getComponent<Sprite>(e).visible = false;
        r.getComponent<Position>(player).x = 100.f;
        r.getComponent<Position>(player).y = 100.f;
        r.getComponent<SpriteSheets>(player).visible = true;
        r.getComponent<Button>(e).enabled = false;
        Input.setControlled(player);
        MessageData msg;
        msg.push_back(static_cast<unsigned char>(0x01));
        const std::string payload = "La game a commencé !";
        msg.insert(msg.end(), payload.begin(), payload.end());
        clientTCP.send(std::move(msg));
        clientUDP.send(std::move(msg));
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

        Input.update(reg, status);

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
