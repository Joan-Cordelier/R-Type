#ifndef ECS_SYSTEMS_STAT_HPP
#define ECS_SYSTEMS_STAT_HPP

#include "../registry.hpp"

class StatSystem {
    public:
        void update(Registry& reg, float dt);
};

#endif