#ifndef ECS_COMPONENTS_ENEMY_HPP
#define ECS_COMPONENTS_ENEMY_HPP

#include <string>

struct Enemy {
    std::string type;
    int health = 100;
    int damage = 10;
    float speed = 50.f;
    float shootInterval = 3.0f;
    float timeSinceLastShot = 0.0f;
    int y_max_position = 800;
    bool canAttack() const { return timeSinceLastShot >= shootInterval; }
};

#endif