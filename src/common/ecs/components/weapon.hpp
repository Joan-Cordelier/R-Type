#pragma once

struct Weapon {
    int damage;
    int nbOfBullets;
    float fireRate;
    float projectileScale = 1.0f;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
};