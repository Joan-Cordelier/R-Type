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
        PollEvent status = renderer.window.pollEvent();
        if (status.type == PollStatus::QUIT)
            break;

        renderer.clear();
        renderer.drawTexture("test", {100, 100, 100, 100});
        renderer.render();
    }

    return 0;
}
