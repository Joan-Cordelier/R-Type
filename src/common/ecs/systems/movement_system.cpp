#include "movement_system.hpp"
#include "../components/position.hpp"
#include "../components/velocity.hpp"
#include "../components/stats.hpp"
#include "../components/projectile.hpp"

void MovementSystem::update(Registry& reg, float dt) {
    auto entities = reg.viewEntitiesWith<Position, Velocity>();
    auto posArr = reg.componentArray<Position>();
    auto velArr = reg.componentArray<Velocity>();
    auto stat = reg.componentArray<Stats>();
    if (!posArr || !velArr || !stat) return;

    for (auto e : entities) {
        if (!posArr->has(e) || !velArr->has(e)) continue;
        if (!stat->has(e)) {
            auto &p = posArr->get(e);
            auto &v = velArr->get(e);
            p.x += v.vx * dt;
            p.y += v.vy * dt;
        } else {
            auto &p = posArr->get(e);
            auto &v = velArr->get(e);
            p.x += v.vx * dt * stat->get(e).movement_speed;
            p.y += v.vy * dt * stat->get(e).movement_speed;
        }
        if (reg.hasComponent<Projectile>(e) &&reg.hasComponent<Position>(e)) {
            Position& position = reg.getComponent<Position>(e);
            if (position.y < -10.f || position.y > 730.f || position.x < -10.f || position.x > 1090.f) {
                reg.destroyEntity(e);
            }
        }
    }
}
