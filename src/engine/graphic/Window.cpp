/*
** EPITECH PROJECT, 2025
** local
** File description:
** Window
*/

#include "Window.hpp"


Window::Window()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    int result = SDL_CreateWindowAndRenderer(1080, 720, 0, &win, &renderer);

    if (result != 0)
        std::cerr << "Failed to create a window and renderer: " << SDL_GetError() << std::endl;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
}

Window::~Window()
{
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (win)
        SDL_DestroyWindow(win);
    SDL_Quit();
}

int Window::draw()
{
    SDL_RenderPresent(renderer);
    return 0;
}

PollStatus Window::pollEvent()
{
    while (SDL_PollEvent(&ev) != 0) {
        switch (ev.type) {
            case SDL_QUIT:
                return PollStatus::QUIT;
        }
    }
    return PollStatus::NONE;
}

void Window::clear()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void Window::clear(Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer);
}