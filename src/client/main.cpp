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
    std::string id = renderer.loadTexture("a.png", "test");

    Uint64 last = SDL_GetPerformanceCounter();
    bool running = true;

    buttonsys.registerHandler("start_game", [&](Registry& r, Entity e) {
        r.getComponent<Label>(label_input).visible = false;
        r.getComponent<Sprite>(e).visible = false;
        r.getComponent<Position>(player).x = 100.f;
        r.getComponent<Sprite>(player).visible = true;
    });

    while (running) {
        SDL_Event evt = renderer.window.pollEvent();
        while (evt.type != 0) {
            switch(evt.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                default:
                    break;
            }
            evt = renderer.window.pollEvent();
        }

        Uint64 now = SDL_GetPerformanceCounter();
        double dt = (double)(now - last) / SDL_GetPerformanceFrequency();
        last = now;

        arrowInput.update(reg);

        movement.update(reg, static_cast<float>(dt));

        buttonsys.update(reg);

        renderer.clear();
        renderer.drawTexture("test", {100, 100, 100, 100}, {.rotation = 45.0f, .alpha = 100, .tint = Color(255, 255, 255)});
        renderer.render();
    }

    return 0;
}
