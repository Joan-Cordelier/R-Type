/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Client main entry point
*/

#include "graphic/Renderer.hpp"
#include "../engine/ecs/registry.hpp"
#include "../engine/ecs/systems/movement_system.hpp"
#include "../engine/ecs/systems/sprite_system.hpp"
#include "../engine/ecs/systems/button_system.hpp"
#include "../engine/ecs/systems/label_system.hpp"
#include "arrow_input_system.hpp"

#include "../engine/ecs/components/position.hpp"
#include "../engine/ecs/components/velocity.hpp"
#include "../engine/ecs/components/sprite.hpp"
#include "../engine/ecs/components/button.hpp"
#include "../engine/ecs/components/label.hpp"

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
    ArrowInputSystem arrowInput(200.f);

    renderer.loadTexture("textures/vaisseau.png", "test");

    Entity player = reg.createEntity();
    reg.addComponent<Position>(player, 100.f, 100.f);
    reg.addComponent<Velocity>(player, 0.f, 0.f);
    reg.addComponent<Sprite>(player, (std::string)"textures/vaisseau.png", (std::string)"test", 64, 64, 0, false);

    renderer.loadTexture("textures/play_button/default.png", "play_button");

    Entity start_button = reg.createEntity();
    reg.addComponent<Position>(start_button, 400.f, 300.f);
    reg.addComponent<Sprite>(start_button, (std::string)"textures/play_button/default.png", (std::string)"play_button", 300, 150, 0, true);
    reg.addComponent<Button>(start_button, (std::string)"start_game", 1, true);

    renderer.loadFont("font/josefin-sans/JosefinSans-Regular.ttf", 24, "default_font");

    Entity label_input = reg.createEntity();
    reg.addComponent<Position>(label_input, 300.f, 300.f);
    reg.addComponent<Label>(label_input, (std::string)"", (std::string)"font/josefin-sans/JosefinSans-Regular.ttf", (std::string)"default_font", Color(255, 255, 255), 0, true);

    arrowInput.setControlled(player);

    Uint64 last = SDL_GetPerformanceCounter();
    bool running = true;

    buttonsys.registerHandler("start_game", [&](Registry& r, Entity e) {
        r.getComponent<Label>(label_input).visible = false;
        r.getComponent<Sprite>(e).visible = false;
        r.getComponent<Position>(player).x = 100.f;
        r.getComponent<Sprite>(player).visible = true;
    });

    while (running) {
        PollEvent status = renderer.window.pollEvent();
        if (status.type == PollStatus::QUIT)
            break;

        Uint64 now = SDL_GetPerformanceCounter();
        double dt = (double)(now - last) / SDL_GetPerformanceFrequency();
        last = now;

        arrowInput.update(reg);

        movement.update(reg, static_cast<float>(dt));

        buttonsys.update(reg);

        renderer.clear();

        spritesys.render(reg, [&](const SpriteSystem::TextureId& tid, int width, int height, int x, int y, int z) {
            renderer.queueDraw(RenderLayer::GAME, z, [&]() {
                renderer.drawTexture(tid, Rect{x, y, width, height});
            });
        });

        labelsys.render(reg, [&](const LabelSystem::TextId& tid, std::string& text, int x, int y, Color color) {
            renderer.drawFont(tid, text, x, y, color);
        });

        renderer.render();
    }

    return 0;
}
