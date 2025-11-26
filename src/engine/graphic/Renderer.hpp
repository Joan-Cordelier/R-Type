/*
** EPITECH PROJECT, 2025
** local
** File description:
** Renderer
*/

#pragma once

#include "Window.hpp"
#include "Rect.hpp"
#include "SDL2/SDL_image.h"
#include <unordered_map>

class Renderer
{
public:
    Window window;
private:
    std::unordered_map<std::string, SDL_Texture*> textureCache;

public:
    ~Renderer();
    void clear();
    void render();
    void drawTexture(const std::string &id, Rect rect);
    std::string loadTexture(const std::string &filePath, const std::string &id = "");
};
