#include "weapon_system.hpp"

#include "../components/weapon.hpp"
#include "../components/position.hpp"
#include "../components/velocity.hpp"
#include "../components/projectile.hpp"

void WeaponSystem::fireWeapon(Registry &reg, Entity entity) {
    if (fireCooldowns.find(entity) != fireCooldowns.end() && fireCooldowns[entity] > 0.0f) {
        return; // Still in cooldown
    }
    Weapon weaponComp = reg.getComponent<Weapon>(entity);

    fireCooldowns[entity] = weaponComp.fireRate;

    if (!isServer) {
        return;
    }
    //server side code
    Entity projectile = reg.createEntity();
    reg.addComponent<Position>(projectile, 0.f, 0.f);
    auto &pos = reg.getComponent<Position>(entity);
    reg.getComponent<Position>(projectile).y = pos.y + projOffsetY;
    reg.getComponent<Position>(projectile).x = pos.x + projOffsetX;
    reg.addComponent<Velocity>(projectile, 0.f, -projSpeed);
    reg.addComponent<Projectile>(projectile, weaponComp.damage, std::string("player"));

    if (projNotifier) {
        projNotifier(entity, projectile);
    }
}


void WeaponSystem::update(Registry& reg, float dt) {
    (void)reg;
    for (auto& [entity, cooldown] : fireCooldowns) {
        cooldown -= dt;
        if (cooldown < 0.0f) {
            cooldown = 0.0f;
        }
    }
}