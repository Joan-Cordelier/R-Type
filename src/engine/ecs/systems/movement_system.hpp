#ifndef ECS_SYSTEMS_MOVEMENT_HPP
#define ECS_SYSTEMS_MOVEMENT_HPP

#include "../registry.hpp"

class MovementSystem {
    public:
        void update(Registry& reg, float dt);
};

#endif