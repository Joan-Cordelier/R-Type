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
#include "Event.hpp"

class Window
{
private:
    SDL_Window *win;
    SDL_Event ev;
public:
    Window();
    ~Window();
    SDL_Renderer *renderer;
    int draw();
    PollEvent pollEvent();
    void clear();
    void clear(Color);
    KeyCode sdlKeyToKeyCode(SDL_Keycode sdlKey);
    MouseButton sdlButtonToMouseButton(Uint8 sdlButton);
};