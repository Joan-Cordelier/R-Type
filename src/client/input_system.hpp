/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ArrowInputSystem
*/

#ifndef INPUT_SYSTEM_HPP
#define INPUT_SYSTEM_HPP

#include "../common/ecs/registry.hpp"
#include "Network/NetworkManager.hpp"
#include <SDL2/SDL.h>

class InputSystem {
public:
    InputSystem();
    ~InputSystem();

    void setControlled(Entity e);
    void update(Registry& reg, SDL_Event& e, NetworkManager& networkManager);

private:
    Entity controlled;
    float last_x, last_y;
};

#endif
