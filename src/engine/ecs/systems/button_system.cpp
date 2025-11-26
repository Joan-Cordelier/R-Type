/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** button_system
*/

#include "button_system.hpp"
#include "../components/button.hpp"
#include "../components/position.hpp"
#include <SDL2/SDL.h>
#include <algorithm>

ButtonSystem::ButtonSystem(): prevMouseDown(false){}

void ButtonSystem::registerHandler(const std::string& name, Handler h) {
    handlers[name] = std::move(h);
}

void ButtonSystem::unregisterHandler(const std::string& name) {
    handlers.erase(name);
}

void ButtonSystem::update(Registry& reg) {
    int mx, my;
    Uint32 buttons = SDL_GetMouseState(&mx, &my);
    bool mouseDown = (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;

    if (!prevMouseDown && mouseDown) {
        auto candidates = reg.viewEntitiesWith<Button, Position>();

        std::sort(candidates.begin(), candidates.end(), [&](Entity a, Entity b) {
            const auto& ba = reg.getComponent<Button>(a);
            const auto& bb = reg.getComponent<Button>(b);
            return ba.z > bb.z;
        });

        for (auto e : candidates) {
            const auto& btn = reg.getComponent<Button>(e);
            if (!btn.enabled || !btn.visible) continue;

            if (!reg.hasComponent<Position>(e)) continue;
            const auto& pos = reg.getComponent<Position>(e);

            float left = pos.x;
            float top = pos.y;
            float right = left + btn.w;
            float bottom = top + btn.h;

            if (mx >= left && mx < right && my >= top && my < bottom) {
                auto it = handlers.find(btn.handler);
                if (it != handlers.end()) {
                    it->second(reg, e);
                }
                break;
            }
        }
    }

    prevMouseDown = mouseDown;
}
