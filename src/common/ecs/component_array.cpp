#include "component_array.hpp"

template<typename T>
ComponentArray<T>::ComponentArray() = default;

template<typename T>
ComponentArray<T>::~ComponentArray() = default;

template<typename T>
void ComponentArray<T>::insert(Entity e, T component) {
    data[e] = std::move(component);
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
const T* ComponentArray<T>::getIf(Entity e) const {
    auto it = data.find(e);
    return it == data.end() ? nullptr : &it->second;
}

template<typename T>
std::vector<Entity> ComponentArray<T>::entities() const {
    std::vector<Entity> out;
    out.reserve(data.size());
    for (auto const &kv : data) out.push_back(kv.first);
    return out;
}

#include "components/position.hpp"
#include "components/velocity.hpp"
#include "components/stats.hpp"
#include "components/sprite.hpp"
#include "components/button.hpp"
#include "components/label.hpp"
#include "components/spritesheet.hpp"
#include "components/enemy.hpp"
#include "components/projectile.hpp"
#include "components/slider.hpp"
#include "components/weapon.hpp"

template class ComponentArray<Position>;
template class ComponentArray<Velocity>;
template class ComponentArray<Stats>;
template class ComponentArray<Sprite>;
template class ComponentArray<Button>;
template class ComponentArray<Label>;
template class ComponentArray<SpriteSheets>;
template class ComponentArray<Enemy>;
template class ComponentArray<Projectile>;
template class ComponentArray<Slider>;
template class ComponentArray<Weapon>;