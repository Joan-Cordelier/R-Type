#ifndef ECS_COMPONENT_ARRAY_HPP
#define ECS_COMPONENT_ARRAY_HPP

#include "i_component_array.hpp"
#include <unordered_map>
#include <memory>

template<typename T>
class ComponentArray : public IComponentArray {
public:
    ComponentArray();
    ~ComponentArray() override;

    void insert(Entity e, T component);
    void remove(Entity e) override;
    bool has(Entity e) const override;
    T& get(Entity e);
    const T* getIf(Entity e) const;
    std::vector<Entity> entities() const override;

private:
    std::unordered_map<Entity, T> data;
};

#endif