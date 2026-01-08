#ifndef ENTITY_TYPE_HPP
#define ENTITY_TYPE_HPP

#include <cstdint>

enum class EntityType : uint8_t {
    PLAYER = 0,
    ENEMY = 1,
    PROJECTILE = 2,
    COMPANION = 3
};

#endif