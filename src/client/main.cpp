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
        PollStatus status = renderer.window.pollEvent();
        if (status == QUIT)
            break;
        
        renderer.drawTexture("test", {100, 100, 100, 100});
        renderer.render();
    }

    return 0;
}
