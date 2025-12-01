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
#include <SDL2/SDL_ttf.h>
#include <unordered_map>
#include <map>
#include <functional>

struct SpriteSheet {
    SDL_Texture* texture;
    int frameWidth;
    int frameHeight;
    int columns;
    int rows;
};

struct DrawOptions {
    float rotation = 0.0f;
    uint8_t alpha = 255;
    Color tint = Color(255, 255, 255);
    SDL_Point* center = nullptr;
    SDL_RendererFlip flip = SDL_FLIP_NONE;
};

struct DrawCommand {
    DrawOptions option = {};
    SDL_Texture *texture;
    Rect srcRect;
    Rect destRect;
    int zIndex;
};

struct CachedFont {
    TTF_Font* font;
    int size;
};

struct CachedText {
    SDL_Texture* texture;
    int width;
    int height;
};

enum class RenderLayer {
    BACKGROUND = 0,
    GAME = 1,
    OVERLAY = 2
};

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

public:
    Renderer();
    ~Renderer();
    void clear();
    void render();
    void queueDraw(RenderLayer layer, int zIndex, std::function<void()> drawCall);
    void drawFont(const std::string &id, const std::string &text, int x, int y, Color color, RenderLayer layer, int z);
    void drawFontAndCache(const std::string &id, const std::string &text, int x, int y, Color color, RenderLayer layer, int z);
    void drawTexture(const std::string &id, RenderLayer layer, int z, Rect rect);
    void drawTexture(const std::string &id, RenderLayer layer, int z, Rect rect, DrawOptions options);
    void drawTextureRegion(const std::string &id, RenderLayer layer, int z, Rect srcRect, Rect rect);
    void drawTextureRegion(const std::string &id, RenderLayer layer, int z, Rect srcRect, Rect rect, DrawOptions options);
    void drawFrame(const std::string &id, int frameIndex, RenderLayer layer, int z, Rect rect);
    void drawFrame(const std::string &id, int frameIndex, RenderLayer layer, int z, Rect rect, DrawOptions options);
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
