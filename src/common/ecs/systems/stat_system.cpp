#include "stat_system.hpp"
#include "../components/stats.hpp"

void StatSystem::update(Registry& reg, float dt) {
    auto statsArr = reg.componentArray<Stats>();
    if (!statsArr) return;

    for (auto e : statsArr->entities()) {
        if (!statsArr->has(e)) continue;
        auto &stats = statsArr->get(e);

        if (stats.cooldown > 0.f) {
            stats.cooldown -= dt;
            if (stats.cooldown < 0.f) {
                stats.cooldown = 0.f;
            }
        }
    }
}