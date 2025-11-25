#ifndef ECS_I_COMPONENT_ARRAY_HPP
#define ECS_I_COMPONENT_ARRAY_HPP

#include "entity_manager.hpp"
#include <vector>

struct IComponentArray {
    virtual ~IComponentArray() = default;
    virtual void remove(Entity e) = 0;
    virtual bool has(Entity e) const = 0;
    virtual std::vector<Entity> entities() const = 0;
};

#endif