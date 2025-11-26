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

struct SpriteSheet {
    SDL_Texture* texture;
    int frameWidth;
    int frameHeight;
    int columns;
    int rows;
};

class Renderer
{
public:
    Window window;
private:
    std::unordered_map<std::string, SDL_Texture*> textureCache;
    std::unordered_map<std::string, SpriteSheet> spritesheetCache;

public:
    ~Renderer();
    void clear();
    void render();
    void drawTexture(const std::string &id, Rect rect);
    void drawTextureRegion(const std::string &id, Rect srcRect, Rect rect);
    void drawFrame(const std::string &id, int frameIndex, Rect rect);
    std::string loadTexture(const std::string &filePath, const std::string &id = "");
    std::string loadSpriteSheet(const std::string &filePath, const std::string &id,
        int frameWidth, int frameHeight, int columns, int rows);
    std::string loadSpriteSheet(const std::string &filePath, const std::string &id,
        int frameWidth, int frameHeight);
    bool isLoaded(const std::string &id) const;
    SpriteSheet getSpritesheetInfo(const std::string &id) const;
    int getFrameCount(const std::string &id) const;
};
