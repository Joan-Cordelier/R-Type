#pragma once

struct Weapon {
    int damage;
    int nbOfBullets;
    float fireRate;
    bool goTowardNearestEnemy = false;
};