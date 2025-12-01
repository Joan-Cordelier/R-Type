/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ArrowInputSystem
*/

#ifndef INPUT_SYSTEM_HPP
#define INPUT_SYSTEM_HPP

#include "../common/ecs/registry.hpp"
#include <SDL2/SDL.h>

class InputSystem {
public:
    explicit InputSystem(float speed);
    ~InputSystem();

    void setControlled(Entity e);
    void update(Registry& reg, SDL_Event& e);

private:
    Entity controlled;
    float speed = 200.f;
};

#endif
