/*
** EPITECH PROJECT, 2025
** local
** File description:
** Renderer
*/

#pragma once

#include "Window.hpp"
#include "Rect.hpp"
#include "RenderingData.hpp"
#include "SDL2/SDL_image.h"
#include <SDL2/SDL_ttf.h>
#include <unordered_map>
#include <map>
#include <functional>

class Renderer
{
public:
    Window window;
private:
    std::unordered_map<std::string, SDL_Texture*> textureCache;
    std::unordered_map<std::string, SpriteSheet> spritesheetCache;
    std::unordered_map<std::string, CachedText> textCache;
    std::unordered_map<std::string, CachedFont> fontCache;

    std::map<RenderLayer, std::vector<DrawCommand>> drawCommands;

    void clearTextCache();
    void invalidateText(const std::string& fontId, const std::string& text);
    std::string makeTextKey(const std::string& fontId, const std::string& text, Color color);
    void renderLine(const DrawCommand& cmd);
    void renderRect(const DrawCommand& cmd);

public:
    Renderer();
    ~Renderer();
    void clear();
    void render();

    void drawFont(const std::string &id, const std::string &text, int x, int y, Color color, RenderLayer layer, int z);
    void drawFontAndCache(const std::string &id, const std::string &text, int x, int y, Color color, RenderLayer layer, int z);
    void drawTexture(const std::string &id, RenderLayer layer, int z, Rect rect);
    void drawTexture(const std::string &id, RenderLayer layer, int z, Rect rect, DrawOptions options);
    void drawTextureRegion(const std::string &id, RenderLayer layer, int z, Rect srcRect, Rect rect);
    void drawTextureRegion(const std::string &id, RenderLayer layer, int z, Rect srcRect, Rect rect, DrawOptions options);
    void drawFrame(const std::string &id, int frameIndex, RenderLayer layer, int z, Rect rect);
    void drawFrame(const std::string &id, int frameIndex, RenderLayer layer, int z, Rect rect, DrawOptions options);
    
    void drawLine(int x1, int y1, int x2, int y2, Color color, RenderLayer layer, int z);
    void drawRect(Rect rect, Color color, RenderLayer layer, int z, bool filled = false);
    
    std::string loadFont(const std::string &filePath, int fontSize, const std::string &id = "");
    std::string loadTexture(const std::string &filePath, const std::string &id = "");
    std::string loadSpriteSheet(const std::string &filePath, const std::string &id,
        int frameWidth, int frameHeight, int columns, int rows);
    std::string loadSpriteSheet(const std::string &filePath, const std::string &id,
        int frameWidth, int frameHeight);

    bool isLoaded(const std::string &id) const;
    SpriteSheet getSpritesheetInfo(const std::string &id) const;
    int getFrameCount(const std::string &id) const;
};
