/*
** EPITECH PROJECT, 2025
** local
** File description:
** ECS
*/

#include "ecs.hpp"
#include <algorithm>

EntityManager::EntityManager() : nextId(1) {}

Entity EntityManager::create() {
    if (!freeIds.empty()) {
        Entity e = freeIds.back();
        freeIds.pop_back();
        return e;
    }
    return nextId++;
}

void EntityManager::destroy(Entity e) {
    freeIds.push_back(e);
}

template<typename T>
void ComponentArray<T>::insert(Entity e, T component) {
    data[e] = component;
}

template<typename T>
void ComponentArray<T>::remove(Entity e) {
    data.erase(e);
}

template<typename T>
bool ComponentArray<T>::has(Entity e) const {
    return data.find(e) != data.end();
}

template<typename T>
T& ComponentArray<T>::get(Entity e) {
    return data.at(e);
}

template<typename T>
std::vector<Entity> ComponentArray<T>::entities() const {
    std::vector<Entity> out;
    out.reserve(data.size());
    for (auto &kv : data)
        out.push_back(kv.first);
    return out;
}

template class ComponentArray<Position>;
template class ComponentArray<Velocity>;
template class ComponentArray<Stats>;

Entity Registry::createEntity() {
    return em.create();
}

void Registry::destroyEntity(Entity e) {
    for (auto &kv : components)
        kv.second->remove(e);
    em.destroy(e);
}

template<typename T>
std::shared_ptr<ComponentArray<T>> Registry::ensure() {
    auto ti = std::type_index(typeid(T));
    auto it = components.find(ti);

    if (it == components.end()) {
        auto arr = std::make_shared<ComponentArray<T>>();
        components[ti] = arr;
        return arr;
    }
    return std::static_pointer_cast<ComponentArray<T>>(it->second);
}

template<typename T, typename... Args>
void Registry::addComponent(Entity e, Args&&... args) {
    ensure<T>()->insert(e, T{std::forward<Args>(args)...});
}

template<typename T>
bool Registry::hasComponent(Entity e) const {
    auto it = components.find(std::type_index(typeid(T)));
    return (it != components.end() && it->second->has(e));
}

template<typename T>
T& Registry::getComponent(Entity e) {
    return std::static_pointer_cast<ComponentArray<T>>(components[std::type_index(typeid(T))])->get(e);
}

template<typename T>
void Registry::collectEntities(std::vector<std::vector<Entity>>& lists) const {
    auto it = components.find(std::type_index(typeid(T)));
    if (it == components.end()) {
        lists.emplace_back();
        return;
    }
    lists.push_back(it->second->entities());
}
