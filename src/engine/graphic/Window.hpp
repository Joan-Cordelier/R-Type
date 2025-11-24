/*
** EPITECH PROJECT, 2025
** local
** File description:
** Window
*/

#include <SDL2/SDL.h>
#include <iostream>
#include "Color.hpp"

enum PollStatus {
    QUIT,
    NONE
};

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
    PollStatus pollEvent();
    void clear();
    void clear(Color);
};