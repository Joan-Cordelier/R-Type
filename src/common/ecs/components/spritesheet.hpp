#ifndef ECS_COMPONENTS_SPRITESHEET_HPP
#define ECS_COMPONENTS_SPRITESHEET_HPP

#include <string>
#include <cstdint>

struct SpriteSheets {
    std::string textureName;
    std::string textureIndex;
    int width = 64;
    int height = 64;
    int frameIndex = 0;
    int maxFrames = 1;
    int z = 0;
    bool visible = true;
    bool loop = false;
};

#endif