#pragma once

#include "../registry.hpp"
#include <map>
#include <functional>

#include "ecs/components/weapon.hpp"

enum class WeaponType {
    DEFAULT,
    SHOTGUN,
    MISSILE
};

class WeaponSystem {
private:
    std::map<Entity, float> fireCooldowns;
    std::function<void(Entity, Entity)> projNotifier;
    bool isServer = true;
    float projOffsetX = 52.0f;
    float projOffsetY = 30.0f;
    float projSpeed = 400.0f;
public:
    void update(Registry& reg, float dt);
    void fireWeapon(Registry &reg, Entity entity);
    void setProjectileNotifier(std::function<void(Entity, Entity)> notifier) {
        projNotifier = notifier;
    }
    void setProjectileConfig(float offsetX, float offsetY, float speed) {
        projOffsetX = offsetX;
        projOffsetY = offsetY;
        projSpeed = speed;
    }
    bool canAttack(Entity entity) const {
        auto it = fireCooldowns.find(entity);
        return it == fireCooldowns.end() || it->second <= 0.0f;
    }
    void resetCooldown(Registry &reg, Entity entity) {
        if (reg.hasComponent<Weapon>(entity)) {
            Weapon weaponComp = reg.getComponent<Weapon>(entity);
            fireCooldowns[entity] = weaponComp.fireRate;
        }
    }
    void setWeaponType(Registry &reg, Entity entity, WeaponType type);
    void setIsServer(bool server) {
        isServer = server;
    }
};