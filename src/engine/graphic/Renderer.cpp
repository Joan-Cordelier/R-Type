/*
** EPITECH PROJECT, 2025
** local
** File description:
** Renderer
*/

#include "Renderer.hpp"
#include <algorithm>

Renderer::Renderer()
    : daltonianMode(DaltonianType::None), daltonianStrength(0.0f),
      renderTarget(nullptr), renderTargetWidth(0), renderTargetHeight(0)
{
    // Constructor can initialize SDL_ttf if needed
    if (TTF_Init() == -1) {
        std::cerr << "TTF_Init failed: " << TTF_GetError() << std::endl;
        exit(84);
    }
}

Renderer::~Renderer()
{
    destroyRenderTarget();
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
    bool useColorblindFilter = (daltonianMode != DaltonianType::None && daltonianStrength > 0.0f);
    
    if (useColorblindFilter) {
        int currentWidth, currentHeight;
        SDL_GetRendererOutputSize(window.renderer, &currentWidth, &currentHeight);
        if (renderTarget == nullptr || renderTargetWidth != currentWidth || renderTargetHeight != currentHeight) {
            createRenderTarget();
        }
        SDL_SetRenderTarget(window.renderer, renderTarget);
        SDL_SetRenderDrawColor(window.renderer, 0, 0, 0, 255);
        SDL_RenderClear(window.renderer);
    }
    
    // Render all commands
    for (auto& [order, commands] : drawCommands) {
        std::stable_sort(commands.begin(), commands.end(),
        [](const DrawCommand& a, const DrawCommand& b) {
            if (a.zIndex != b.zIndex)
                return a.zIndex < b.zIndex;
            return a.texture < b.texture;
        });
        for (auto& cmd : commands) {
            if (cmd.texture != nullptr && cmd.type == DrawType::Texture) {
                SDL_SetTextureColorMod(cmd.texture, cmd.option.tint.r, cmd.option.tint.g, cmd.option.tint.b);
                SDL_SetTextureAlphaMod(cmd.texture, cmd.option.alpha);
                SDL_SetTextureBlendMode(cmd.texture, cmd.option.blendMode);
            
                SDL_Rect destRect = cmd.destRect.toSDLRect();
                SDL_Rect* srcRect = nullptr;
                SDL_Rect srcRectObj;
                if (!cmd.srcRect.isNull()) {
                    srcRectObj = cmd.srcRect.toSDLRect();
                    srcRect = &srcRectObj;
                }
                SDL_RenderCopyEx(window.renderer, cmd.texture, srcRect, &destRect, 
                    cmd.option.rotation, cmd.option.center, cmd.option.flip);
            }
            else {
                if (cmd.type == DrawType::Texture) {
                    std::cerr << "Attempted to draw a null texture." << std::endl;
                }
                switch(cmd.type) {
                    case DrawType::Line:
                        renderLine(cmd);
                        break;
                    case DrawType::Rect:
                        renderRect(cmd);
                        break;
                    default:
                        break;
                }
            }
        }
    }
    drawCommands.clear();
    
    // Apply colorblind filter
    if (useColorblindFilter) {
        SDL_SetRenderTarget(window.renderer, nullptr);
        
        // Interpolate between no filter and colorblind tint based on strength
        Color filterColor = getColorblindTintColor();
        uint8_t r = static_cast<uint8_t>(255 + (filterColor.r - 255) * daltonianStrength);
        uint8_t g = static_cast<uint8_t>(255 + (filterColor.g - 255) * daltonianStrength);
        uint8_t b = static_cast<uint8_t>(255 + (filterColor.b - 255) * daltonianStrength);
        
        SDL_SetTextureBlendMode(renderTarget, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(renderTarget, 255);
        SDL_SetTextureColorMod(renderTarget, r, g, b);
        SDL_RenderCopy(window.renderer, renderTarget, nullptr, nullptr);
    }
    
    window.draw();
}

void Renderer::drawFont(const std::string &id, const std::string &text, int x, int y, Color color, RenderLayer layer, int z)
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

    Rect destRect = {x, y, surface->w, surface->h};
    SDL_FreeSurface(surface);
    
    DrawCommand cmd = {{}, texture, {}, destRect, z};
    drawCommands[layer].push_back(cmd);
}

/// @brief draw a font and cache the rendered text
/// @param id the id of the font
/// @param text the text to render
/// @param x the x position
/// @param y the y position
/// @param color the color of the text
void Renderer::drawFontAndCache(const std::string &id, const std::string &text, 
                        int x, int y, Color color, RenderLayer layer, int z)
{
    std::string cacheKey = makeTextKey(id, text, color);
    
    // Check cache first
    auto it = textCache.find(cacheKey);
    if (it != textCache.end()) {
        Rect destRect = {x, y, it->second.width, it->second.height};
        DrawCommand cmd = {{}, it->second.texture, {}, destRect, z};
        drawCommands[layer].push_back(cmd);
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
    
    Rect destRect = {x, y, surface->w, surface->h};
    SDL_FreeSurface(surface);
    
    DrawCommand cmd = {{}, texture, {}, destRect, z};
    drawCommands[layer].push_back(cmd);
}

/// @brief draw a texture to the screen
/// @param id the id of the texture
/// @param rect the destination rectangle
void Renderer::drawTexture(const std::string &id, RenderLayer layer, int z, Rect rect)
{
    drawTexture(id, layer, z, rect, DrawOptions{});
}

/// @brief draw a texture to the screen
/// @param id the id of the texture
/// @param rect the destination rectangle
/// @param options drawing options like rotation and scale
void Renderer::drawTexture(const std::string &id, RenderLayer layer, int z, Rect rect, DrawOptions options)
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

    DrawCommand cmd = {options, texture, {}, rect, z};
    drawCommands[layer].push_back({cmd});
}

/// @brief draw a region of a texture
/// @param id the id of the texture
/// @param srcRect the source rectangle
/// @param rect the destination rectangle
void Renderer::drawTextureRegion(const std::string &id, RenderLayer layer, int z, Rect srcRect, Rect rect)
{
    drawTextureRegion(id, layer, z, srcRect, rect, DrawOptions{});
}

/// @brief draw a region of a texture
/// @param id the id of the texture
/// @param srcRect the source rectangle
/// @param rect the destination rectangle
/// @param options drawing options like rotation and scale
void Renderer::drawTextureRegion(const std::string &id, RenderLayer layer, int z, Rect srcRect, Rect rect, DrawOptions options)
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

    DrawCommand cmd = {options, texture, srcRect, rect, z};
    drawCommands[layer].push_back({cmd});
}

/// @brief draw a specific frame from a spritesheet
/// @param id the id of the spritesheet
/// @param frameIndex the index of the frame to draw
/// @param rect the destination rectangle
void Renderer::drawFrame(const std::string &id, int frameIndex, RenderLayer layer, int z, Rect rect)
{
    drawFrame(id, frameIndex, layer, z, rect, DrawOptions{});
}

/// @brief draw a specific frame from a spritesheet
/// @param id the id of the spritesheet
/// @param frameIndex the index of the frame to draw
/// @param rect the destination rectangle
/// @param options drawing options like rotation and scale
void Renderer::drawFrame(const std::string &id, int frameIndex, RenderLayer layer, int z, Rect rect, DrawOptions options)
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

    Rect srcRect = {
        (frameIndex % sheet.columns) * sheet.frameWidth,
        (frameIndex / sheet.columns) * sheet.frameHeight,
        sheet.frameWidth,
        sheet.frameHeight
    };

    DrawCommand cmd = {options, texture, srcRect, rect, z};
    drawCommands[layer].push_back(cmd);
}

void Renderer::drawLine(int x1, int y1, int x2, int y2, Color color, RenderLayer layer, int z)
{
    DrawCommand cmd = {{.tint = color}, nullptr, {}, {}, z, DrawType::Line, {x1, y1, x2, y2, false}};
    drawCommands[layer].push_back(cmd);
}

void Renderer::drawRect(Rect rect, Color color, RenderLayer layer, int z, bool filled)
{
    DrawCommand cmd = {{.tint = color}, nullptr, {}, {}, z, DrawType::Rect, {rect.x, rect.y, rect.w, rect.h, filled}};
    drawCommands[layer].push_back(cmd);
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

void Renderer::renderLine(const DrawCommand& cmd) {
    SDL_SetRenderDrawColor(window.renderer, cmd.option.tint.r, cmd.option.tint.g, cmd.option.tint.b, 255);
    SDL_RenderDrawLine(window.renderer, cmd.primitiveData.x1, cmd.primitiveData.y1,
                       cmd.primitiveData.x2, cmd.primitiveData.y2);
}

void Renderer::renderRect(const DrawCommand& cmd) {
    SDL_SetRenderDrawColor(window.renderer, cmd.option.tint.r, cmd.option.tint.g, cmd.option.tint.b, 255);
    SDL_Rect sdlRect = {cmd.primitiveData.x1, cmd.primitiveData.y1,
                        cmd.primitiveData.x2, cmd.primitiveData.y2};
    if (cmd.primitiveData.filled) {
        SDL_RenderFillRect(window.renderer, &sdlRect);
    } else {
        SDL_RenderDrawRect(window.renderer, &sdlRect);
    }
}

void Renderer::setDaltonianMode(DaltonianType type, float strength) {
    daltonianMode = type;
    daltonianStrength = std::max(0.0f, std::min(100.0f, strength)) / 100.0f;
}

DaltonianType Renderer::getDaltonianMode() const {
    return daltonianMode;
}

float Renderer::getDaltonianStrength() const {
    return daltonianStrength * 100.0f;
}

Color Renderer::applyDaltonianFilter(const Color& color) const {
    if (daltonianMode == DaltonianType::None || daltonianStrength == 0.0f) {
        return color;
    }

    // Convert to normalized RGB (0.0 - 1.0)
    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;

    float newR, newG, newB;

    // Apply color transformation matrix based on type
    switch (daltonianMode) {
        case DaltonianType::Protanopia:
            // Protanopia simulation (missing red cones)
            newR = 0.567f * r + 0.433f * g;
            newG = 0.558f * r + 0.442f * g;
            newB = 0.242f * g + 0.758f * b;
            break;

        case DaltonianType::Deuteranopia:
            // Deuteranopia simulation (missing green cones)
            newR = 0.625f * r + 0.375f * g;
            newG = 0.700f * r + 0.300f * g;
            newB = 0.300f * g + 0.700f * b;
            break;

        case DaltonianType::Tritanopia:
            // Tritanopia simulation (missing blue cones)
            newR = 0.950f * r + 0.050f * g;
            newG = 0.433f * g + 0.567f * b;
            newB = 0.475f * g + 0.525f * b;
            break;

        default:
            newR = r;
            newG = g;
            newB = b;
            break;
    }

    // Interpolate between original and simulated colors based on strength
    float finalR = r + (newR - r) * daltonianStrength;
    float finalG = g + (newG - g) * daltonianStrength;
    float finalB = b + (newB - b) * daltonianStrength;

    // Clamp and convert back to 0-255 range
    uint8_t outR = static_cast<uint8_t>(std::max(0.0f, std::min(255.0f, finalR * 255.0f)));
    uint8_t outG = static_cast<uint8_t>(std::max(0.0f, std::min(255.0f, finalG * 255.0f)));
    uint8_t outB = static_cast<uint8_t>(std::max(0.0f, std::min(255.0f, finalB * 255.0f)));

    return Color(outR, outG, outB, color.a);
}

void Renderer::createRenderTarget() {
    destroyRenderTarget();
    
    SDL_GetRendererOutputSize(window.renderer, &renderTargetWidth, &renderTargetHeight);
    renderTarget = SDL_CreateTexture(window.renderer, SDL_PIXELFORMAT_RGBA8888,
                                     SDL_TEXTUREACCESS_TARGET, renderTargetWidth, renderTargetHeight);
    if (!renderTarget) {
        std::cerr << "Failed to create render target: " << SDL_GetError() << std::endl;
    }
}

void Renderer::destroyRenderTarget() {
    if (renderTarget) {
        SDL_DestroyTexture(renderTarget);
        renderTarget = nullptr;
    }
}

Color Renderer::getColorblindTintColor() const {
    switch (daltonianMode) {
        case DaltonianType::Protanopia:
            return Color(140, 180, 255);
        
        case DaltonianType::Deuteranopia:
            return Color(180, 140, 255);
        
        case DaltonianType::Tritanopia:
            return Color(255, 180, 140);
        
        default:
            return Color(255, 255, 255);
    }
}