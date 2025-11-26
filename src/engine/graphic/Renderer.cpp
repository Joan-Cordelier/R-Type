/*
** EPITECH PROJECT, 2025
** local
** File description:
** Renderer
*/

#include "Renderer.hpp"

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
}

/// @brief clear the window
void Renderer::clear()
{
    window.clear();
}

/// @brief present the rendered content to the window
void Renderer::render()
{
    window.draw();
}

/// @brief draw a texture to the screen
/// @param id the id of the texture
/// @param rect the destination rectangle
void Renderer::drawTexture(const std::string &id, Rect rect)
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

    SDL_SetRenderDrawColor(window.renderer, 255, 255, 255, 255);
    SDL_RenderCopy(window.renderer, texture, NULL, &destRect);
}

/// @brief draw a region of a texture
/// @param id the id of the texture
/// @param srcRect the source rectangle
/// @param rect the destination rectangle
void Renderer::drawTextureRegion(const std::string &id, Rect srcRect, Rect rect)
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
    SDL_Rect texRect = {srcRect.x, srcRect.y, srcRect.w, srcRect.h};

    SDL_SetRenderDrawColor(window.renderer, 255, 255, 255, 255);
    SDL_RenderCopy(window.renderer, texture, &texRect, &destRect);
}

/// @brief draw a specific frame from a spritesheet
/// @param id the id of the spritesheet
/// @param frameIndex the index of the frame to draw
/// @param rect the destination rectangle
void Renderer::drawFrame(const std::string &id, int frameIndex, Rect rect)
{
    if (spritesheetCache.find(id) == spritesheetCache.end()) {
        std::cerr << "Error on spritesheet loading for drawing: " << id << " not found." << std::endl;
        return;
    }

    SDL_Texture *texture = spritesheetCache[id].texture;
    if (texture == nullptr) {
        std::cerr << "Error on spritesheet loading for drawing: " << id << " not initialized." << std::endl;
        return;
    }

    SDL_Rect destRect = rect.toSDLRect();
    SDL_Rect texRect = {
        (frameIndex % spritesheetCache[id].columns) * spritesheetCache[id].frameWidth,
        (frameIndex / spritesheetCache[id].columns) * spritesheetCache[id].frameHeight,
        spritesheetCache[id].frameWidth,
        spritesheetCache[id].frameHeight
    };

    SDL_SetRenderDrawColor(window.renderer, 255, 255, 255, 255);
    SDL_RenderCopy(window.renderer, texture, &texRect, &destRect);
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