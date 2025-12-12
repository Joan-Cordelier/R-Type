#include "enemy_system.hpp"
#include "../components/enemy.hpp"
#include "../components/position.hpp"
#include "../components/velocity.hpp"
#include "../components/projectile.hpp"

void EnemySystem::update(Registry& reg, float dt) {
    newProjectileEntitiesWithParent.clear();
    newEnemyEntities.clear();
    projectileColliding.clear();
    for (auto e : reg.viewEntitiesWith<Enemy, Position, Velocity>()) {
        Position& position = reg.getComponent<Position>(e);
        Velocity& velocity = reg.getComponent<Velocity>(e);
        Enemy& enemy = reg.getComponent<Enemy>(e);
        if (position.y > enemy.y_max_position) {
            velocity.vy = 0.f;
        }
        if (enemy.canAttack()) {
            Entity projectile = reg.createEntity();
            reg.addComponent<Position>(projectile, position.x - 25.f, position.y - 50.f);
            reg.addComponent<Velocity>(projectile, 0.f, 200.f);
            reg.addComponent<Projectile>(projectile, enemy.damage, (std::string)"enemy");
            newProjectileEntitiesWithParent.push_back(std::make_pair(e, projectile));
            enemy.timeSinceLastShot = 0.0f;
        } else {
            enemy.timeSinceLastShot += dt;
        }
        for (auto& entity : reg.viewEntitiesWith<Projectile, Position, Velocity>()) {
            if (reg.getComponent<Projectile>(entity).ownerType == "player") {
                Position& projPos = reg.getComponent<Position>(entity);
                if (projPos.x >= position.x && projPos.x <= position.x + 50 &&
                    projPos.y >= position.y && projPos.y <= position.y + 50) {
                    enemy.health -= reg.getComponent<Projectile>(entity).damage;
                    projectileColliding.push_back(entity);
                    if (enemy.health <= 0) {
                        reg.destroyEntity(e);
                        enemiesAlive--;
                    }
                }
            }
        }
    }
    
    if (enemiesAlive <= 0 && canSpawn) {
        wave++;
        enemiesAlive = wave;
        for (int i = 0; i < enemiesAlive; ++i) {
            Entity enemyEntity = reg.createEntity();
            reg.addComponent<Position>(enemyEntity, static_cast<float>(rand() % 800), -50.f);
            reg.addComponent<Velocity>(enemyEntity, 0.f, 50.f);
            reg.addComponent<Enemy>(enemyEntity, (std::string)"basic", 100, 10, 50.f, 3.0f, 0.0f, (50 + rand() % 150));
            enemyEntities.push_back(enemyEntity);
            newEnemyEntities.push_back(enemyEntity);
        }
    }
}
