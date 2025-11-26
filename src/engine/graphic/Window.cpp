/*
** EPITECH PROJECT, 2025
** local
** File description:
** Window
*/

#include "Window.hpp"


Window::Window()
{
    int initresult = SDL_Init(SDL_INIT_EVERYTHING);
    if (initresult != 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        exit(84);
    }
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

SDL_Event Window::pollEvent()
{
    std::lock_guard<std::mutex> lock(eventMutex);
    
    if (eventQueue.empty()) {
        processSDLEvents();
    }
    
    if (eventQueue.empty()) {
        SDL_Event noneEvent;
        noneEvent.type = 0;
        return noneEvent;
    }
    
    SDL_Event event = eventQueue.front();
    eventQueue.pop();
    return event;
}

void Window::processSDLEvents()
{
    SDL_PumpEvents();
    
    SDL_Event sdlEvent;
    while (SDL_PeepEvents(&sdlEvent, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT) == 1) {
        eventQueue.push(sdlEvent);
    }
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