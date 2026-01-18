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
    float score = 100.f; // Points awarded when killed

    // Movement pattern parameters
    float move_amplitude = 0.0f;
    float move_frequency = 0.0f;
    float time_alive = 0.0f;    // To calculate sine wave position
    float max_lifetime = -1.0f; // < 0 means infinite/managed externally

    bool canAttack() const {
        return timeSinceLastShot >= shootInterval;
    }
};

#endif