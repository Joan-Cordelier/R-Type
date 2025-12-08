#include "movement_system.hpp"
#include "../components/position.hpp"
#include "../components/velocity.hpp"
#include "../components/stats.hpp"

void MovementSystem::update(Registry& reg, float dt) {
    auto entities = reg.viewEntitiesWith<Position, Velocity>();
    auto posArr = reg.componentArray<Position>();
    auto velArr = reg.componentArray<Velocity>();
    auto stat = reg.componentArray<Stats>();
    if (!posArr || !velArr) return;

    for (auto e : entities) {
        if (!posArr->has(e) || !velArr->has(e)) continue;
        auto &p = posArr->get(e);
        auto &v = velArr->get(e);
        p.x += v.vx * dt * stat->get(e).movement_speed;
        p.y += v.vy * dt * stat->get(e).movement_speed;
    }
}