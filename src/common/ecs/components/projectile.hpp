#ifndef ECS_COMPONENTS_PROJECTILE_HPP
#define ECS_COMPONENTS_PROJECTILE_HPP

#include <string>

struct Projectile {
    int damage = 10;
    std::string ownerType; // "player" or "enemy"
};

#endif