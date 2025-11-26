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
    std::string idFont = renderer.loadFont("font.ttf", 16, "mainFont");

    bool running = true;

    while (running) {
        PollEvent status = renderer.window.pollEvent();
        if (status.type == PollStatus::QUIT)
            break;

        renderer.clear();
        renderer.drawTexture("test", {100, 100, 100, 100}, {.rotation = 45.0f, .alpha = 100, .tint = Color(255, 255, 255)});
        renderer.drawFont("mainFont", "Anto gay !", 50, 50, Color(255, 255, 255));
        renderer.render();
    }

    return 0;
}
