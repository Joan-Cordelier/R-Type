#include "registry.hpp"

#include "components/button.hpp"
#include "components/enemy.hpp"
#include "components/label.hpp"
#include "components/parent.hpp"
#include "components/position.hpp"
#include "components/projectile.hpp"
#include "components/slider.hpp"
#include "components/sprite.hpp"
#include "components/spritesheet.hpp"
#include "components/stats.hpp"
#include "components/velocity.hpp"
#include "components/weapon.hpp"

#include <algorithm>
#include <type_traits>

Registry::Registry() = default;

Entity Registry::createEntity() {
    return em.create();
}

void Registry::destroyEntity(Entity e) {
    for (auto &kv : components)
        kv.second->remove(e);
    em.destroy(e);
}

template <typename T> std::shared_ptr<ComponentArray<T>> Registry::ensure() {
    auto ti = std::type_index(typeid(T));
    auto it = components.find(ti);
    if (it == components.end()) {
        auto arr = std::make_shared<ComponentArray<T>>();
        components.emplace(ti, arr);
        return arr;
    }
    return std::static_pointer_cast<ComponentArray<T>>(it->second);
}

template <typename T> std::shared_ptr<ComponentArray<T>> Registry::componentArray() const {
    auto it = components.find(std::type_index(typeid(T)));
    if (it == components.end())
        return nullptr;
    return std::static_pointer_cast<ComponentArray<T>>(it->second);
}

template <typename T, typename... Args> void Registry::addComponent(Entity e, Args... args) {
    ensure<T>()->insert(e, T{std::forward<Args>(args)...});
}

template <typename T> bool Registry::hasComponent(Entity e) const {
    auto it = components.find(std::type_index(typeid(T)));
    return (it != components.end() && it->second->has(e));
}

template <typename T> T &Registry::getComponent(Entity e) {
    return std::static_pointer_cast<ComponentArray<T>>(components.at(std::type_index(typeid(T))))
        ->get(e);
}

template <typename T>
void Registry::collectEntities(std::vector<std::vector<Entity>> &lists) const {
    auto it = components.find(std::type_index(typeid(T)));
    if (it == components.end()) {
        lists.emplace_back();
        return;
    }
    lists.push_back(it->second->entities());
}

template <typename... Ts> std::vector<Entity> Registry::viewEntitiesWith() const {
    std::vector<std::vector<Entity>> lists;
    (collectEntities<Ts>(lists), ...);

    if (lists.empty())
        return {};

    std::sort(lists.begin(), lists.end(),
              [](auto const &a, auto const &b) { return a.size() < b.size(); });

    std::vector<Entity> out = lists.front();
    for (size_t i = 1; i < lists.size(); ++i) {
        std::vector<Entity> tmp;
        tmp.reserve(std::min(out.size(), lists[i].size()));
        for (Entity e : out) {
            if (std::find(lists[i].begin(), lists[i].end(), e) != lists[i].end())
                tmp.push_back(e);
        }
        out.swap(tmp);
        if (out.empty())
            break;
    }

    return out;
}

template std::shared_ptr<ComponentArray<Position>> Registry::ensure<Position>();
template std::shared_ptr<ComponentArray<Velocity>> Registry::ensure<Velocity>();
template std::shared_ptr<ComponentArray<Stats>> Registry::ensure<Stats>();
template std::shared_ptr<ComponentArray<Sprite>> Registry::ensure<Sprite>();
template std::shared_ptr<ComponentArray<Button>> Registry::ensure<Button>();
template std::shared_ptr<ComponentArray<Label>> Registry::ensure<Label>();
template std::shared_ptr<ComponentArray<SpriteSheets>> Registry::ensure<SpriteSheets>();
template std::shared_ptr<ComponentArray<Enemy>> Registry::ensure<Enemy>();
template std::shared_ptr<ComponentArray<Projectile>> Registry::ensure<Projectile>();
template std::shared_ptr<ComponentArray<Slider>> Registry::ensure<Slider>();
template std::shared_ptr<ComponentArray<Weapon>> Registry::ensure<Weapon>();

template std::shared_ptr<ComponentArray<Position>> Registry::componentArray<Position>() const;
template std::shared_ptr<ComponentArray<Velocity>> Registry::componentArray<Velocity>() const;
template std::shared_ptr<ComponentArray<Stats>> Registry::componentArray<Stats>() const;
template std::shared_ptr<ComponentArray<Sprite>> Registry::componentArray<Sprite>() const;
template std::shared_ptr<ComponentArray<Button>> Registry::componentArray<Button>() const;
template std::shared_ptr<ComponentArray<Label>> Registry::componentArray<Label>() const;
template std::shared_ptr<ComponentArray<SpriteSheets>>
Registry::componentArray<SpriteSheets>() const;
template std::shared_ptr<ComponentArray<Enemy>> Registry::componentArray<Enemy>() const;
template std::shared_ptr<ComponentArray<Projectile>> Registry::componentArray<Projectile>() const;
template std::shared_ptr<ComponentArray<Slider>> Registry::componentArray<Slider>() const;
template std::shared_ptr<ComponentArray<Weapon>> Registry::componentArray<Weapon>() const;

template void Registry::addComponent<Position, float, float>(Entity, float, float);
template void Registry::addComponent<Velocity, float, float>(Entity, float, float);
template void Registry::addComponent<Stats, int, int, int, float, int, int, int>(Entity, int, int,
                                                                                 int, float, int,
                                                                                 int, int);
template void
Registry::addComponent<Sprite, std::string, std::string, int, int, int, float, float, bool>(
    Entity, std::string, std::string, int, int, int, float, float, bool);
template void Registry::addComponent<Button, std::string, int, bool>(Entity, std::string, int,
                                                                     bool);
template void
Registry::addComponent<Label, std::string, std::string, std::string, Color, int, bool>(
    Entity, std::string, std::string, std::string, Color, int, bool);
template void Registry::addComponent<SpriteSheets, std::string, std::string, int, int, int, int,
                                     int, float, float, bool, bool>(Entity, std::string,
                                                                    std::string, int, int, int, int,
                                                                    int, float, float, bool, bool);
template void
Registry::addComponent<Enemy, std::string, int, int, float, float, float, int, float, float, float>(
    Entity, std::string, int, int, float, float, float, int, float, float, float);
template void Registry::addComponent<Enemy, std::string, int, int, float, float, float, int, float,
                                     float, float, float>(Entity, std::string, int, int, float,
                                                          float, float, int, float, float, float,
                                                          float);
template void Registry::addComponent<Projectile, int, std::string>(Entity, int, std::string);
template void Registry::addComponent<Projectile, int, std::string, float>(Entity, int, std::string,
                                                                          float);
template void
Registry::addComponent<Slider, float, float, float, float, float, int, int, int, bool, bool, bool,
                       std::string, Color, Color, Color>(Entity, float, float, float, float, float,
                                                         int, int, int, bool, bool, bool,
                                                         std::string, Color, Color, Color);
template void Registry::addComponent<Weapon, int, int, float>(Entity, int, int, float);
template void Registry::addComponent<Weapon, int, int, float, float, float, float>(Entity, int, int,
                                                                                   float, float,
                                                                                   float, float);

template bool Registry::hasComponent<Position>(Entity) const;
template bool Registry::hasComponent<Velocity>(Entity) const;
template bool Registry::hasComponent<Stats>(Entity) const;
template bool Registry::hasComponent<Sprite>(Entity) const;
template bool Registry::hasComponent<Button>(Entity) const;
template bool Registry::hasComponent<Label>(Entity) const;
template bool Registry::hasComponent<SpriteSheets>(Entity) const;
template bool Registry::hasComponent<Enemy>(Entity) const;
template bool Registry::hasComponent<Projectile>(Entity) const;
template bool Registry::hasComponent<Slider>(Entity) const;
template bool Registry::hasComponent<Weapon>(Entity) const;

template Position &Registry::getComponent<Position>(Entity);
template Velocity &Registry::getComponent<Velocity>(Entity);
template Stats &Registry::getComponent<Stats>(Entity);
template Sprite &Registry::getComponent<Sprite>(Entity);
template Button &Registry::getComponent<Button>(Entity);
template Label &Registry::getComponent<Label>(Entity);
template SpriteSheets &Registry::getComponent<SpriteSheets>(Entity);
template Enemy &Registry::getComponent<Enemy>(Entity);
template Projectile &Registry::getComponent<Projectile>(Entity);
template Slider &Registry::getComponent<Slider>(Entity);
template Weapon &Registry::getComponent<Weapon>(Entity);

template void Registry::collectEntities<Position>(std::vector<std::vector<Entity>> &lists) const;
template void Registry::collectEntities<Velocity>(std::vector<std::vector<Entity>> &lists) const;
template void Registry::collectEntities<Stats>(std::vector<std::vector<Entity>> &lists) const;
template void Registry::collectEntities<Sprite>(std::vector<std::vector<Entity>> &lists) const;
template void Registry::collectEntities<Button>(std::vector<std::vector<Entity>> &lists) const;
template void Registry::collectEntities<Label>(std::vector<std::vector<Entity>> &lists) const;
template void
Registry::collectEntities<SpriteSheets>(std::vector<std::vector<Entity>> &lists) const;
template void Registry::collectEntities<Enemy>(std::vector<std::vector<Entity>> &lists) const;
template void Registry::collectEntities<Projectile>(std::vector<std::vector<Entity>> &lists) const;
template void Registry::collectEntities<Slider>(std::vector<std::vector<Entity>> &lists) const;
template void Registry::collectEntities<Weapon>(std::vector<std::vector<Entity>> &lists) const;

template std::vector<Entity> Registry::viewEntitiesWith<Position, Velocity>() const;
template std::vector<Entity> Registry::viewEntitiesWith<Sprite>() const;
template std::vector<Entity> Registry::viewEntitiesWith<Position>() const;
template std::vector<Entity> Registry::viewEntitiesWith<Button>() const;
template std::vector<Entity> Registry::viewEntitiesWith<Button, Position>() const;
template std::vector<Entity> Registry::viewEntitiesWith<Label>() const;
template std::vector<Entity> Registry::viewEntitiesWith<Label, Position>() const;
template std::vector<Entity> Registry::viewEntitiesWith<SpriteSheets>() const;
template std::vector<Entity> Registry::viewEntitiesWith<SpriteSheets, Position>() const;
template std::vector<Entity> Registry::viewEntitiesWith<Enemy>() const;
template std::vector<Entity> Registry::viewEntitiesWith<Enemy, Position>() const;
template std::vector<Entity> Registry::viewEntitiesWith<Enemy, Position, Velocity>() const;
template std::vector<Entity> Registry::viewEntitiesWith<Projectile>() const;
template std::vector<Entity> Registry::viewEntitiesWith<Projectile, Position, Velocity>() const;
template std::vector<Entity> Registry::viewEntitiesWith<Slider>() const;
template std::vector<Entity> Registry::viewEntitiesWith<Slider, Position>() const;
template std::vector<Entity> Registry::viewEntitiesWith<Stats, Position>() const;
template std::vector<Entity> Registry::viewEntitiesWith<Weapon>() const;
template std::vector<Entity> Registry::viewEntitiesWith<Projectile, Position>() const;

template std::shared_ptr<ComponentArray<Parent>> Registry::ensure<Parent>();
template std::shared_ptr<ComponentArray<Parent>> Registry::componentArray<Parent>() const;
template void Registry::addComponent<Parent, Entity, float, float>(Entity, Entity, float, float);
template bool Registry::hasComponent<Parent>(Entity) const;
template Parent &Registry::getComponent<Parent>(Entity);
template void Registry::collectEntities<Parent>(std::vector<std::vector<Entity>> &lists) const;

template std::vector<Entity> Registry::viewEntitiesWith<Parent>() const;
template std::vector<Entity> Registry::viewEntitiesWith<Parent, Position>() const;
template std::vector<Entity> Registry::viewEntitiesWith<Parent, Weapon>() const;
