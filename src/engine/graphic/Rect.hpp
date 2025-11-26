/*
** EPITECH PROJECT, 2025
** local
** File description:
** Rect
*/

#pragma once

class Rect
{
public:
    int x;
    int y;
    int w;
    int h;

    Rect(int x, int y, int w, int h)
    {
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
    }

    SDL_Rect toSDLRect() const
    {
        SDL_Rect rect;
        rect.x = x;
        rect.y = y;
        rect.w = w;
        rect.h = h;
        return rect;
    }

};