/*
** EPITECH PROJECT, 2025
** local
** File description:
** Renderer
*/

#include "Renderer.hpp"

void Renderer::render()
{
    window.draw();
}

void Renderer::drawTexture(const std::string &id, Rect rect)
{
    if (textureCache.find(id) == textureCache.end()) {
        std::cerr << "Error on texture loading for drawing: " << id << " not found.";
        return;
    }
    
    SDL_Texture *texture = textureCache[id];
    if (texture == nullptr) {
        std::cerr << "Error on texture loading for drawing: " << id << " not initialized.";
        return;
    }

    SDL_Rect destRect = {rect.x, rect.y, rect.w, rect.h};

    SDL_SetRenderDrawColor(window.renderer, 255, 255, 255, 255);
    SDL_RenderCopy(window.renderer, texture, NULL, &destRect);
}

std::string Renderer::loadTexture(const std::string &filePath, const std::string &id)
{
    std::string textureId = (id.empty()) ? filePath : id;
    if (textureId.empty())
        return "";

    if (textureCache.find(textureId) != textureCache.end())
        return textureId;
    
    SDL_Surface *surface = IMG_Load(filePath.c_str());
    if (surface == nullptr) {
        std::cerr << "Failed to load image: " << SDL_GetError() << " !";
        return "";
    }
    
    SDL_Texture *texture = SDL_CreateTextureFromSurface(window.renderer, surface);
    if (texture == nullptr) {
        std::cerr << "Failed to create texture from image: " << SDL_GetError() << " !";
        return "";
    }

    SDL_FreeSurface(surface);
    textureCache[textureId] = texture;
    return textureId;
}