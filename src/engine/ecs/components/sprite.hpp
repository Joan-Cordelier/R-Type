#ifndef ECS_COMPONENTS_SPRITE_HPP
#define ECS_COMPONENTS_SPRITE_HPP

#include <string>
#include <cstdint>

struct Sprite {
    std::string textureName;
    std::string textureIndex = "0";
    int z = 0;
    bool visible = true;
};

#endif