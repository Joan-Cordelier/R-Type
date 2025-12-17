#ifndef ECS_ENTITY_MANAGER_HPP
#define ECS_ENTITY_MANAGER_HPP

#include <cstdint>
#include <vector>

using Entity = uint32_t;
constexpr Entity INVALID_ENTITY = 0;

class EntityManager {
public:
    EntityManager();
    Entity create();
    void destroy(Entity e);

private:
    Entity nextId;
    std::vector<Entity> freeIds;
};

#endif