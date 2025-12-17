#ifndef ECS_COMPONENTS_STATS_HPP
#define ECS_COMPONENTS_STATS_HPP

struct Stats {
    int hp = 0;
    int maxHp = 0;
    int attack_speed = 0;
    float cooldown = 0.f;
    int attack_damage = 0;
    int nb_projectiles = 0;
    int movement_speed = 0;
    bool canAttack() const { return cooldown <= 0.f; }
};

#endif