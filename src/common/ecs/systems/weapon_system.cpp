#include "weapon_system.hpp"

#include "../components/weapon.hpp"
#include "../components/position.hpp"
#include "../components/velocity.hpp"
#include "../components/projectile.hpp"
#include "../components/enemy.hpp"
#include <cmath>
#include <limits>

#include <iostream>

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
    float spawnX = pos.x + weaponComp.offsetX;
    float spawnY = pos.y + weaponComp.offsetY;
    
    reg.getComponent<Position>(projectile).x = spawnX;
    reg.getComponent<Position>(projectile).y = spawnY;
    reg.addComponent<Velocity>(projectile, 0.f, -projSpeed);
    reg.addComponent<Projectile>(projectile, weaponComp.damage, std::string("player"), weaponComp.projectileScale);

    if (projNotifier) {
        projNotifier(entity, projectile);
    }

    if (weaponComp.nbOfBullets > 1) {
        static int offsetPerBullet = 20;
        int remaining = weaponComp.nbOfBullets - 1;
        for (int i = 0; i < remaining / 2; ++i) {
            Entity projLeft = reg.createEntity();
            reg.addComponent<Position>(projLeft, spawnX - (offsetPerBullet * (i + 1)), spawnY);
            reg.addComponent<Velocity>(projLeft, 0.0f, -projSpeed);
            reg.addComponent<Projectile>(projLeft, weaponComp.damage, std::string("player"), weaponComp.projectileScale);
            Entity projRight = reg.createEntity();
            reg.addComponent<Position>(projRight, spawnX + (offsetPerBullet * (i + 1)), spawnY);
            reg.addComponent<Velocity>(projRight, 0.0f, -projSpeed);
            reg.addComponent<Projectile>(projRight, weaponComp.damage, std::string("player"), weaponComp.projectileScale);
            if (projNotifier) {
                projNotifier(entity, projRight);
                projNotifier(entity, projLeft);
            }
        }
    }

}

void WeaponSystem::setWeaponType(Registry &reg, Entity entity, WeaponType type) {
    Weapon weapon;
    switch (type) {
        case WeaponType::DEFAULT: //TODO: Harcoded for now need to use yaml config value
            weapon.damage = 10;
            weapon.nbOfBullets = 1;
            weapon.fireRate = 0.5f;
            break;
        case WeaponType::SHOTGUN:
            weapon.damage = 7;
            weapon.nbOfBullets = 3;
            weapon.fireRate = 1.0f;
            break;
        case WeaponType::MISSILE:
            weapon.damage = 45;
            weapon.nbOfBullets = 1;
            weapon.fireRate = 1.9f;
            break;
        default:
            weapon.damage = 10;
            weapon.nbOfBullets = 1;
            weapon.fireRate = 0.5f;
            break;
    }
    
    weapon.offsetX = projOffsetX;
    weapon.offsetY = projOffsetY;

    if (reg.hasComponent<Weapon>(entity)) {
        reg.getComponent<Weapon>(entity) = weapon;
    } else {
        reg.addComponent<Weapon>(entity, weapon.damage, weapon.nbOfBullets, weapon.fireRate, weapon.projectileScale, weapon.offsetX, weapon.offsetY);
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