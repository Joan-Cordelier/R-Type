#include "movement_system.hpp"
#include "../components/position.hpp"
#include "../components/velocity.hpp"

void MovementSystem::update(Registry& reg, float dt) {
    auto entities = reg.viewEntitiesWith<Position, Velocity>();
    auto posArr = reg.componentArray<Position>();
    auto velArr = reg.componentArray<Velocity>();
    if (!posArr || !velArr) return;

    for (auto e : entities) {
        if (!posArr->has(e) || !velArr->has(e)) continue;
        auto &p = posArr->get(e);
        auto &v = velArr->get(e);
        p.x += v.vx * dt;
        p.y += v.vy * dt;
    }
}