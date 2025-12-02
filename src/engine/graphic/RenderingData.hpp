#pragma once

#include "SDL2/SDL_image.h"
#include <SDL2/SDL_ttf.h>
#include "Rect.hpp"
#include "Color.hpp"

enum class DrawType {
    Texture,
    Line,
    Rect
};

enum class RenderLayer {
    BACKGROUND = 0,
    GAME = 1,
    OVERLAY = 2
};

struct SpriteSheet {
    SDL_Texture* texture;
    int frameWidth;
    int frameHeight;
    int columns;
    int rows;
};

struct PrimitiveData {
    int x1, y1, x2, y2;
    bool filled = false;
};

struct CachedFont {
    TTF_Font* font;
    int size;
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
    SDL_Texture *texture = nullptr;
    Rect srcRect;
    Rect destRect;
    int zIndex;
    DrawType type = DrawType::Texture;
    PrimitiveData primitiveData = {};
};

struct CachedText {
    SDL_Texture* texture;
    int width;
    int height;
};