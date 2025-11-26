/*
** EPITECH PROJECT, 2025
** local
** File description:
** Renderer
*/

#include "Renderer.hpp"

Renderer::Renderer()
{
    // Constructor can initialize SDL_ttf if needed
    if (TTF_Init() == -1) {
        std::cerr << "TTF_Init failed: " << TTF_GetError() << std::endl;
        exit(84);
    }
}

Renderer::~Renderer()
{
    for (auto &pair : textureCache) {
        if (pair.second) {
            SDL_DestroyTexture(pair.second);
        }
    }
    for (auto &pair : spritesheetCache) {
        if (pair.second.texture) {
            SDL_DestroyTexture(pair.second.texture);
        }
    }
    for (auto &pair : fontCache) {
        if (pair.second.font) {
            TTF_CloseFont(pair.second.font);
        }
    }
    for (auto& pair : textCache) {
        SDL_DestroyTexture(pair.second.texture);
    }
    TTF_Quit();
}

/// @brief clear the window
void Renderer::clear()
{
    window.clear();
}

/// @brief present the rendered content to the window
void Renderer::render()
{
    for (auto& [layer, commands] : drawLayers) {
        std::stable_sort(commands.begin(), commands.end(),
            [](const DrawCommand& a, const DrawCommand& b) {
                return a.zIndex < b.zIndex;
            });
        for (auto& cmd : commands) {
            cmd.command();
        }
    }
    drawLayers.clear();
    window.draw();
}

/// @brief add a draw call to the queue
/// @param layer layer to draw on
/// @param zIndex index for draw order
/// @param drawCall the draw function to call
void Renderer::queueDraw(RenderLayer layer, int zIndex, std::function<void()> drawCall)
{
    drawLayers[layer].push_back({zIndex, drawCall});
}

void Renderer::drawFont(const std::string &id, const std::string &text, int x, int y, Color color)
{
    if (fontCache.find(id) == fontCache.end()) {
        std::cerr << "Font not found: " << id << std::endl;
        return;
    }
    
    TTF_Font* font = fontCache[id].font;
    SDL_Color sdlColor = {color.r, color.g, color.b, color.a};
    
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), sdlColor);
    if (surface == nullptr) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(window.renderer, surface);
    if (texture == nullptr) {
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect destRect = {x, y, surface->w, surface->h};
    SDL_FreeSurface(surface);
    
    SDL_RenderCopy(window.renderer, texture, NULL, &destRect);
    SDL_DestroyTexture(texture);
}

/// @brief draw a font and cache the rendered text
/// @param id the id of the font
/// @param text the text to render
/// @param x the x position
/// @param y the y position
/// @param color the color of the text
void Renderer::drawFontAndCache(const std::string &id, const std::string &text, 
                        int x, int y, Color color)
{
    std::string cacheKey = makeTextKey(id, text, color);
    
    // Check cache first
    auto it = textCache.find(cacheKey);
    if (it != textCache.end()) {
        SDL_Rect destRect = {x, y, it->second.width, it->second.height};
        SDL_RenderCopy(window.renderer, it->second.texture, NULL, &destRect);
        return;
    }
    
    // Cache miss - create texture
    TTF_Font* font = fontCache[id].font;
    SDL_Color sdlColor = {color.r, color.g, color.b, color.a};
    
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), sdlColor);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(window.renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }
    
    // Store in cache
    textCache[cacheKey] = {texture, surface->w, surface->h};
    
    SDL_Rect destRect = {x, y, surface->w, surface->h};
    SDL_FreeSurface(surface);
    
    SDL_RenderCopy(window.renderer, texture, NULL, &destRect);
}

/// @brief draw a texture to the screen
/// @param id the id of the texture
/// @param rect the destination rectangle
void Renderer::drawTexture(const std::string &id, Rect rect)
{
    drawTexture(id, rect, DrawOptions{});
}

/// @brief draw a texture to the screen
/// @param id the id of the texture
/// @param rect the destination rectangle
/// @param options drawing options like rotation and scale
void Renderer::drawTexture(const std::string &id, Rect rect, DrawOptions options)
{
    if (textureCache.find(id) == textureCache.end()) {
        std::cerr << "Error on texture loading for drawing: " << id << " not found." << std::endl;;
        return;
    }
    
    SDL_Texture *texture = textureCache[id];
    if (texture == nullptr) {
        std::cerr << "Error on texture loading for drawing: " << id << " not initialized." << std::endl;
        return;
    }

    SDL_SetTextureColorMod(texture, options.tint.r, options.tint.g, options.tint.b);
    SDL_SetTextureAlphaMod(texture, options.alpha);

    SDL_Rect destRect = rect.toSDLRect();

    SDL_RenderCopyEx(window.renderer, texture, NULL, &destRect, 
                     options.rotation, options.center, options.flip);
}

/// @brief draw a region of a texture
/// @param id the id of the texture
/// @param srcRect the source rectangle
/// @param rect the destination rectangle
void Renderer::drawTextureRegion(const std::string &id, Rect srcRect, Rect rect)
{
    drawTextureRegion(id, srcRect, rect, DrawOptions{});
}

/// @brief draw a region of a texture
/// @param id the id of the texture
/// @param srcRect the source rectangle
/// @param rect the destination rectangle
/// @param options drawing options like rotation and scale
void Renderer::drawTextureRegion(const std::string &id, Rect srcRect, Rect rect, DrawOptions options)
{
    if (textureCache.find(id) == textureCache.end()) {
        std::cerr << "Error on texture loading for drawing: " << id << " not found." << std::endl;;
        return;
    }
    
    SDL_Texture *texture = textureCache[id];
    if (texture == nullptr) {
        std::cerr << "Error on texture loading for drawing: " << id << " not initialized." << std::endl;
        return;
    }

    SDL_Rect destRect = rect.toSDLRect();
    SDL_Rect texRect = srcRect.toSDLRect();

    SDL_SetTextureColorMod(texture, options.tint.r, options.tint.g, options.tint.b);
    SDL_SetTextureAlphaMod(texture, options.alpha);

    SDL_RenderCopyEx(window.renderer, texture, &texRect, &destRect,
                     options.rotation, options.center, options.flip);
}

/// @brief draw a specific frame from a spritesheet
/// @param id the id of the spritesheet
/// @param frameIndex the index of the frame to draw
/// @param rect the destination rectangle
void Renderer::drawFrame(const std::string &id, int frameIndex, Rect rect)
{
    drawFrame(id, frameIndex, rect, DrawOptions{});
}

/// @brief draw a specific frame from a spritesheet
/// @param id the id of the spritesheet
/// @param frameIndex the index of the frame to draw
/// @param rect the destination rectangle
/// @param options drawing options like rotation and scale
void Renderer::drawFrame(const std::string &id, int frameIndex, Rect rect, DrawOptions options)
{
    if (spritesheetCache.find(id) == spritesheetCache.end()) {
        std::cerr << "Error on spritesheet loading for drawing: " << id << " not found." << std::endl;
        return;
    }
    SpriteSheet &sheet = spritesheetCache[id];

    SDL_Texture *texture = sheet.texture;
    if (texture == nullptr) {
        std::cerr << "Error on spritesheet loading for drawing: " << id << " not initialized." << std::endl;
        return;
    }

    SDL_SetTextureColorMod(texture, options.tint.r, options.tint.g, options.tint.b);
    SDL_SetTextureAlphaMod(texture, options.alpha);

    SDL_Rect destRect = rect.toSDLRect();
    SDL_Rect texRect = {
        (frameIndex % sheet.columns) * sheet.frameWidth,
        (frameIndex / sheet.columns) * sheet.frameHeight,
        sheet.frameWidth,
        sheet.frameHeight
    };

    SDL_RenderCopyEx(window.renderer, texture, &texRect, &destRect,
                     options.rotation, options.center, options.flip);
}

/// @brief load a spritesheet from file
/// @param filePath the filepath of the spritesheet
/// @param id the id to assign to the spritesheet
/// @param frameWidth the width of each frame
/// @param frameHeight the height of each frame
/// @param columns the number of columns in the spritesheet
/// @param rows the number of rows in the spritesheet
/// @return the id of the loaded spritesheet, or empty string on failure
std::string Renderer::loadSpriteSheet(const std::string &filePath, const std::string &id,
        int frameWidth, int frameHeight, int columns, int rows)
{
    std::string textureId = (id.empty()) ? filePath : id;
    if (textureId.empty())
        return "";

    if (spritesheetCache.find(textureId) != spritesheetCache.end())
        return textureId;
    
    SDL_Surface *surface = IMG_Load(filePath.c_str());
    if (surface == nullptr) {
        std::cerr << "Failed to load image: " << SDL_GetError() << " !" << std::endl;
        return "";
    }
    
    SDL_Texture *texture = SDL_CreateTextureFromSurface(window.renderer, surface);
    if (texture == nullptr) {
        std::cerr << "Failed to create texture from image: " << SDL_GetError() << " !" << std::endl;
        SDL_FreeSurface(surface);
        return "";
    }

    SDL_FreeSurface(surface);
    spritesheetCache[textureId] = {texture, frameWidth, frameHeight, columns, rows};
    return textureId;
}

/// @brief load a spritesheet from file with automatic columns and rows calculation
/// @param filePath the filepath of the spritesheet
/// @param id the id to assign to the spritesheet
/// @param frameWidth the width of each frame
/// @param frameHeight  the height of each frame
/// @return the id of the loaded spritesheet, or empty string on failure
std::string Renderer::loadSpriteSheet(const std::string &filePath, const std::string &id,
        int frameWidth, int frameHeight)
{
    std::string textureId = (id.empty()) ? filePath : id;
    if (textureId.empty())
        return "";

    if (spritesheetCache.find(textureId) != spritesheetCache.end())
        return textureId;
    
    SDL_Surface *surface = IMG_Load(filePath.c_str());
    if (surface == nullptr) {
        std::cerr << "Failed to load image: " << SDL_GetError() << " !" << std::endl;
        return "";
    }
    
    SDL_Texture *texture = SDL_CreateTextureFromSurface(window.renderer, surface);
    if (texture == nullptr) {
        std::cerr << "Failed to create texture from image: " << SDL_GetError() << " !" << std::endl;
        return "";
    }

    int texWidth, texHeight;
    SDL_QueryTexture(texture, NULL, NULL, &texWidth, &texHeight);
    int columns = texWidth / frameWidth;
    int rows = texHeight / frameHeight;

    SDL_FreeSurface(surface);
    spritesheetCache[textureId] = {texture, frameWidth, frameHeight, columns, rows};
    return textureId;
}

std::string Renderer::loadFont(const std::string &filePath, int fontSize, const std::string &id)
{
    std::string fontId = (id.empty()) ? filePath : id;
    if (fontId.empty())
        return "";

    if (fontCache.find(fontId) != fontCache.end())
        return fontId;

    TTF_Font* font = TTF_OpenFont(filePath.c_str(), fontSize);
    if (font == nullptr) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return "";
    }

    fontCache[fontId] = {font, fontSize};
    return fontId;
}

/// @brief load a texture from file
/// @param filePath the filepath of the texture
/// @param id the id to assign to the texture (optional)
/// @return the id of the loaded texture, or empty string on failure
std::string Renderer::loadTexture(const std::string &filePath, const std::string &id)
{
    std::string textureId = (id.empty()) ? filePath : id;
    if (textureId.empty())
        return "";

    if (textureCache.find(textureId) != textureCache.end())
        return textureId;
    
    SDL_Surface *surface = IMG_Load(filePath.c_str());
    if (surface == nullptr) {
        std::cerr << "Failed to load image: " << SDL_GetError() << " !" << std::endl;
        return "";
    }
    
    SDL_Texture *texture = SDL_CreateTextureFromSurface(window.renderer, surface);
    if (texture == nullptr) {
        std::cerr << "Failed to create texture from image: " << SDL_GetError() << " !" << std::endl;
        return "";
    }

    SDL_FreeSurface(surface);
    textureCache[textureId] = texture;
    return textureId;
}

/// @brief return if a texture or spritesheet is loaded
/// @param id id of the texture or spritesheet
/// @return a boolean indicating if it's loaded
bool Renderer::isLoaded(const std::string &id) const
{
    return (textureCache.find(id) != textureCache.end() ||
            spritesheetCache.find(id) != spritesheetCache.end());
}

/// @brief return the spritesheet info
/// @param id the id of the spritesheet
/// @return the spritesheet info, or a default one if not found
SpriteSheet Renderer::getSpritesheetInfo(const std::string &id) const
{
    if (spritesheetCache.find(id) != spritesheetCache.end()) {
        return spritesheetCache.at(id);
    }
    std::cerr << "Spritesheet not found: " << id << std::endl;
    return {nullptr, 0, 0, 0, 0};
}

/// @brief return the number of frames in the spritesheet
/// @param id the id of the spritesheet
/// @return the number of frames, or 0 if not found
int Renderer::getFrameCount(const std::string &id) const
{
    if (spritesheetCache.find(id) != spritesheetCache.end()) {
        const SpriteSheet &sheet = spritesheetCache.at(id);
        return sheet.columns * sheet.rows;
    }
    std::cerr << "Spritesheet not found: " << id << std::endl;
    return 0;
}

std::string Renderer::makeTextKey(const std::string& fontId, const std::string& text, Color color) {
    std::string key;
    key.reserve(fontId.size() + text.size() + 20);
    key += fontId;
    key += '|';
    key += text;
    key += '|';
    key += std::to_string(color.r);
    key += std::to_string(color.g);
    key += std::to_string(color.b);
    key += std::to_string(color.a);
    return key;
}