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
#include "KeybindsManager.hpp"
#include "../common/ecs/systems/weapon_system.hpp"

class InputSystem {
public:
    InputSystem();
    ~InputSystem();

    void setControlled(Entity e, KeybindsManager& kbManager);
    void update(Registry& reg, SDL_Event& e, NetworkManager& networkManager, WeaponSystem& weaponsys);

private:
    Entity controlled;
    KeybindsManager* keybindsManager;
    float last_x, last_y;
};

#endif
