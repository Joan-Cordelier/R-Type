#include "registry.hpp"

#include "components/position.hpp"
#include "components/velocity.hpp"
#include "components/stats.hpp"
#include "components/sprite.hpp"
#include "components/button.hpp"

#include <algorithm>
#include <type_traits>

Registry::Registry() = default;

Entity Registry::createEntity() {
    return em.create();
}

void Registry::destroyEntity(Entity e) {
    for (auto &kv : components) kv.second->remove(e);
    em.destroy(e);
}

template<typename T>
std::shared_ptr<ComponentArray<T>> Registry::ensure() {
    auto ti = std::type_index(typeid(T));
    auto it = components.find(ti);
    if (it == components.end()) {
        auto arr = std::make_shared<ComponentArray<T>>();
        components.emplace(ti, arr);
        return arr;
    }
    return std::static_pointer_cast<ComponentArray<T>>(it->second);
}

template<typename T>
std::shared_ptr<ComponentArray<T>> Registry::componentArray() const {
    auto it = components.find(std::type_index(typeid(T)));
    if (it == components.end()) return nullptr;
    return std::static_pointer_cast<ComponentArray<T>>(it->second);
}

template<typename T, typename... Args>
void Registry::addComponent(Entity e, Args... args) {
    ensure<T>()->insert(e, T{std::forward<Args>(args)...});
}

template<typename T>
bool Registry::hasComponent(Entity e) const {
    auto it = components.find(std::type_index(typeid(T)));
    return (it != components.end() && it->second->has(e));
}

template<typename T>
T& Registry::getComponent(Entity e) {
    return std::static_pointer_cast<ComponentArray<T>>(components.at(std::type_index(typeid(T))))->get(e);
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

template<typename...Ts>
std::vector<Entity> Registry::viewEntitiesWith() const {
    std::vector<std::vector<Entity>> lists;
    (collectEntities<Ts>(lists), ...);

    if (lists.empty()) return {};

    std::sort(lists.begin(), lists.end(),
              [](auto const &a, auto const &b){ return a.size() < b.size(); });

    std::vector<Entity> out = lists.front();
    for (size_t i = 1; i < lists.size(); ++i) {
        std::vector<Entity> tmp;
        tmp.reserve(std::min(out.size(), lists[i].size()));
        for (Entity e : out) {
            if (std::find(lists[i].begin(), lists[i].end(), e) != lists[i].end())
                tmp.push_back(e);
        }
        out.swap(tmp);
        if (out.empty()) break;
    }

    return out;
}

template std::shared_ptr<ComponentArray<Position>> Registry::ensure<Position>();
template std::shared_ptr<ComponentArray<Velocity>> Registry::ensure<Velocity>();
template std::shared_ptr<ComponentArray<Stats>> Registry::ensure<Stats>();
template std::shared_ptr<ComponentArray<Sprite>> Registry::ensure<Sprite>();
template std::shared_ptr<ComponentArray<Button>> Registry::ensure<Button>();

template std::shared_ptr<ComponentArray<Position>> Registry::componentArray<Position>() const;
template std::shared_ptr<ComponentArray<Velocity>> Registry::componentArray<Velocity>() const;
template std::shared_ptr<ComponentArray<Stats>> Registry::componentArray<Stats>() const;
template std::shared_ptr<ComponentArray<Sprite>> Registry::componentArray<Sprite>() const;
template std::shared_ptr<ComponentArray<Button>> Registry::componentArray<Button>() const;

template void Registry::addComponent<Position, float, float>(Entity, float, float);
template void Registry::addComponent<Velocity, float, float>(Entity, float, float);
template void Registry::addComponent<Stats, int>(Entity, int);
template void Registry::addComponent<Sprite, std::string, std::string, int, bool>(Entity, std::string, std::string, int, bool);
template void Registry::addComponent<Button, float, float, std::string, int, bool>(Entity, float, float, std::string, int, bool);

template bool Registry::hasComponent<Position>(Entity) const;
template bool Registry::hasComponent<Velocity>(Entity) const;
template bool Registry::hasComponent<Stats>(Entity) const;
template bool Registry::hasComponent<Sprite>(Entity) const;
template bool Registry::hasComponent<Button>(Entity) const;

template Position& Registry::getComponent<Position>(Entity);
template Velocity& Registry::getComponent<Velocity>(Entity);
template Stats& Registry::getComponent<Stats>(Entity);
template Sprite& Registry::getComponent<Sprite>(Entity);
template Button& Registry::getComponent<Button>(Entity);

template void Registry::collectEntities<Position>(std::vector<std::vector<Entity>>& lists) const;
template void Registry::collectEntities<Velocity>(std::vector<std::vector<Entity>>& lists) const;
template void Registry::collectEntities<Stats>(std::vector<std::vector<Entity>>& lists) const;
template void Registry::collectEntities<Sprite>(std::vector<std::vector<Entity>>& lists) const;
template void Registry::collectEntities<Button>(std::vector<std::vector<Entity>>& lists) const;

template std::vector<Entity> Registry::viewEntitiesWith<Position, Velocity>() const;
template std::vector<Entity> Registry::viewEntitiesWith<Sprite>() const;
template std::vector<Entity> Registry::viewEntitiesWith<Position>() const;
template std::vector<Entity> Registry::viewEntitiesWith<Button>() const;
template std::vector<Entity> Registry::viewEntitiesWith<Button, Position>() const;
