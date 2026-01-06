#pragma once

#include "../registry.hpp"
#include <map>
#include <functional>

class WeaponSystem {
private:
    std::map<Entity, float> fireCooldowns;
    std::function<void(Entity, Entity)> projNotifier;
    bool isServer = true;
public:
    void update(Registry& reg, float dt);
    void fireWeapon(Registry &reg, Entity entity);
    void setProjectileNotifier(std::function<void(Entity, Entity)> notifier) {
        projNotifier = notifier;
    }
    bool canAttack(Entity entity) const {
        auto it = fireCooldowns.find(entity);
        return it == fireCooldowns.end() || it->second <= 0.0f;
    }
    void setIsServer(bool server) {
        isServer = server;
    }
};