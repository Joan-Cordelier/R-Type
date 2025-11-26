/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ArrowInputSystem
*/

#pragma once

#include "../engine/ecs/registry.hpp"
#include <cstdint>

class ArrowInputSystem {
public:
    explicit ArrowInputSystem(float speed = 200.f);
    void setControlled(Entity e);
    void update(Registry& reg);

private:
    Entity controlled;
    float speed;
};
