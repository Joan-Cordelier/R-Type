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
        return;
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
    

    int mode = weaponComp.nbOfBullets;
    int parallelCount = mode;
    bool isDiagonal = false;

    if (mode >= 100) {
        isDiagonal = true;
        parallelCount = mode - 100;
        if (parallelCount < 1) parallelCount = 1;
    }

    // Parallel Shots (Center is already fired, so we spawn parallelCount - 1)
    if (parallelCount > 1) {
        static int offsetPerBullet = 20;
        int remaining = parallelCount - 1;
        
        for (int k = 0; k < remaining; ++k) {
            int pairIndex = (k / 2) + 1;
            bool isLeft = (k % 2) == 0;
            
            Entity extraProj = reg.createEntity();
            float offsetX = isLeft ? - (offsetPerBullet * pairIndex) : (offsetPerBullet * pairIndex);
            
            reg.addComponent<Position>(extraProj, spawnX + offsetX, spawnY);
            reg.addComponent<Velocity>(extraProj, 0.0f, -projSpeed);
            reg.addComponent<Projectile>(extraProj, weaponComp.damage, std::string("player"), weaponComp.projectileScale);
            
            if (projNotifier) {
                projNotifier(entity, extraProj);
            }
        }
    }

    // Diagonal Shots
    if (isDiagonal) {
        float diagVY = -projSpeed * 0.9f;
        float diagVX = projSpeed * 0.3f; // ~18 degrees
        
        Entity projDiagL = reg.createEntity();
        reg.addComponent<Position>(projDiagL, spawnX - 20, spawnY);
        reg.addComponent<Velocity>(projDiagL, -diagVX, diagVY);
        reg.addComponent<Projectile>(projDiagL, weaponComp.damage, std::string("player"), weaponComp.projectileScale);

        Entity projDiagR = reg.createEntity();
        reg.addComponent<Position>(projDiagR, spawnX + 20, spawnY);
        reg.addComponent<Velocity>(projDiagR, diagVX, diagVY);
        reg.addComponent<Projectile>(projDiagR, weaponComp.damage, std::string("player"), weaponComp.projectileScale);
        
        if (projNotifier) {
            projNotifier(entity, projDiagL);
            projNotifier(entity, projDiagR);
        }
    }
}

void WeaponSystem::setWeaponType(Registry &reg, Entity entity, WeaponType type) {
    Weapon weapon;
    switch (type) {
        case WeaponType::DEFAULT:
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