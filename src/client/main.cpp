/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Client main entry point
*/

#include "graphic/Renderer.hpp"

int main(void)
{
    Renderer renderer;
    std::string id = renderer.loadTexture("a.png", "test");

    bool running = true;

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

        renderer.clear();
        renderer.drawTexture("test", {100, 100, 100, 100}, {.rotation = 45.0f, .alpha = 100, .tint = Color(255, 255, 255)});
        renderer.render();
    }

    return 0;
}
