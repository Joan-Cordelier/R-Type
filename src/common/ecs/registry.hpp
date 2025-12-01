#ifndef ECS_REGISTRY_HPP
#define ECS_REGISTRY_HPP

#include "entity_manager.hpp"
#include "i_component_array.hpp"
#include "component_array.hpp"

#include <typeindex>
#include <memory>
#include <unordered_map>
#include <vector>

class Registry {
public:
    Registry();

    Entity createEntity();
    void destroyEntity(Entity e);
    template<typename T>
    std::shared_ptr<ComponentArray<T>> ensure();

    template<typename T>
    std::shared_ptr<ComponentArray<T>> componentArray() const;
    template<typename T, typename... Args>
    void addComponent(Entity e, Args... args);

    template<typename T>
    bool hasComponent(Entity e) const;

    template<typename T>
    T& getComponent(Entity e);
    template<typename...Ts>
    std::vector<Entity> viewEntitiesWith() const;

private:
    EntityManager em;
    std::unordered_map<std::type_index, std::shared_ptr<IComponentArray>> components;

    template<typename T>
    void collectEntities(std::vector<std::vector<Entity>>& lists) const;
};

#endif