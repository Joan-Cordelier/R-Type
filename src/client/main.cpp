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
#include "arrow_input_system.hpp"

#include "../engine/ecs/components/position.hpp"
#include "../engine/ecs/components/velocity.hpp"
#include "../engine/ecs/components/sprite.hpp"

#include <functional>
#include <SDL2/SDL.h>

int main()
{
    Renderer renderer;
    Registry reg;
    MovementSystem movement;
    SpriteSystem spritesys;
    ArrowInputSystem arrowInput(200.f);

    renderer.loadTexture("textures/vaisseau.png", "test");
    spritesys.registerTexture("textures/vaisseau.png", "test");

    Entity player = reg.createEntity();
    reg.addComponent<Position>(player, 100.f, 100.f);
    reg.addComponent<Velocity>(player, 0.f, 0.f);
    reg.addComponent<Sprite>(player, (std::string)"textures/vaisseau.png", (std::string)"test", 0, true);

    arrowInput.setControlled(player);

    Uint64 last = SDL_GetPerformanceCounter();
    bool running = true;
    while (running) {
        PollEvent status = renderer.window.pollEvent();
        if (status.type == PollStatus::QUIT)
            break;

        Uint64 now = SDL_GetPerformanceCounter();
        double dt = (double)(now - last) / SDL_GetPerformanceFrequency();
        last = now;

        arrowInput.update(reg);

        movement.update(reg, static_cast<float>(dt));

        renderer.clear();

        spritesys.render(reg, [&](SpriteSystem::TextureId tid, float x, float y, int z) {
            (void)z; // on va l'utiliser plus tard pour le système de calque
            renderer.drawTexture(tid, { static_cast<int>(x), static_cast<int>(y), 64, 64 });
        });

        renderer.render();
    }

    return 0;
}
