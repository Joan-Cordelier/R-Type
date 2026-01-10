#ifndef ECS_COMPONENTS_SPRITE_HPP
#define ECS_COMPONENTS_SPRITE_HPP

#include <string>
#include <cstdint>

struct Sprite {
    std::string textureName;
    std::string textureIndex;
    int width = 64;
    int height = 64;
    int z = 0;
    float offset_x = 0;
    float offset_y = 0;
    bool visible = true;
};

#endif