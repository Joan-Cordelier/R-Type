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
    auto &pos = reg.getComponent<Position>(entity);
    
    // Find nearest enemy if weapon targets enemies
    int nearestEnemy = -1;
    float minDistance = std::numeric_limits<float>::max();
    
    if (weaponComp.goTowardNearestEnemy) {
        auto enemies = reg.viewEntitiesWith<Enemy, Position>();
        for (Entity e : enemies) {
            auto &enemyPos = reg.getComponent<Position>(e);
            float dx = enemyPos.x - pos.x;
            float dy = enemyPos.y - pos.y;
            float distance = std::sqrt(dx * dx + dy * dy);
            
            if (distance < minDistance) {
                minDistance = distance;
                nearestEnemy = static_cast<int>(e);
            }
        }
    }
    
    // Calculate spread angles for multiple bullets
    float coneAngle = 45.0f * (3.14159265f / 180.0f); // 45 degrees in radians
    int numBullets = weaponComp.nbOfBullets;
    
    for (int i = 0; i < numBullets; ++i) {
        Entity projectile = reg.createEntity();
        reg.addComponent<Position>(projectile, 0.f, 0.f);
        reg.getComponent<Position>(projectile).y = pos.y + projOffsetY;
        reg.getComponent<Position>(projectile).x = pos.x + projOffsetX;
        
        float velocityX = 0.f;
        float velocityY = -projSpeed;
        
        // Handle spread for multiple bullets
        if (numBullets > 1) {
            // Calculate angle offset for this bullet
            float angleOffset;
            if (numBullets == 1) {
                angleOffset = 0.0f;
            } else {
                // Distribute bullets evenly across the cone
                angleOffset = -coneAngle / 2.0f + (coneAngle / (numBullets - 1)) * i;
            }
            
            // Rotate the default velocity by the angle offset
            float baseVelX = 0.f;
            float baseVelY = -projSpeed;
            velocityX = baseVelX * std::cos(angleOffset) - baseVelY * std::sin(angleOffset);
            velocityY = baseVelX * std::sin(angleOffset) + baseVelY * std::cos(angleOffset);
        }
        
        // Override velocity if targeting nearest enemy
        if (weaponComp.goTowardNearestEnemy && nearestEnemy != -1) {
            auto &enemyPos = reg.getComponent<Position>(nearestEnemy);
            float dx = enemyPos.x - (pos.x + projOffsetX);
            float dy = enemyPos.y - (pos.y + projOffsetY);
            float distance = std::sqrt(dx * dx + dy * dy);
            
            if (distance > 0.0f) {
                // Normalize and scale to projectile speed
                velocityX = (dx / distance) * projSpeed;
                velocityY = (dy / distance) * projSpeed;
            }
        }
        
        reg.addComponent<Velocity>(projectile, velocityX, velocityY);
        reg.addComponent<Projectile>(projectile, weaponComp.damage, std::string("player"));

        if (projNotifier) {
            projNotifier(entity, projectile);
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
            weapon.goTowardNearestEnemy = false;
            break;
        case WeaponType::SHOTGUN:
            weapon.damage = 8;
            weapon.nbOfBullets = 5;
            weapon.fireRate = 1.0f;
            weapon.goTowardNearestEnemy = false;
            break;
        case WeaponType::MISSILE:
            weapon.damage = 3;
            weapon.nbOfBullets = 2;
            weapon.fireRate = 0.4f;
            weapon.goTowardNearestEnemy = true;
            break;
        default:
            weapon.damage = 10;
            weapon.nbOfBullets = 1;
            weapon.fireRate = 0.5f;
            weapon.goTowardNearestEnemy = false;
            break;
    }
    if (reg.hasComponent<Weapon>(entity)) {
        reg.getComponent<Weapon>(entity) = weapon;
    } else {
        reg.addComponent<Weapon>(entity, weapon.damage, weapon.nbOfBullets, weapon.fireRate, false);
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