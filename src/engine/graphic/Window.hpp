/*
** EPITECH PROJECT, 2025
** local
** File description:
** Window
*/

#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <iostream>
#include "Color.hpp"
#include <queue>
#include <mutex>

class Window
{
private:
    SDL_Window *win;
    SDL_Event ev;
public:
    std::queue<SDL_Event> eventQueue;
    std::mutex eventMutex;

    Window();
    ~Window();
    SDL_Renderer *renderer;
    int draw();
    SDL_Event pollEvent();
    void processSDLEvents();
    void clear();
    void clear(Color);
};