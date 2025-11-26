/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ArrowInputSystem
*/

#include "input_system.hpp"
#include "../engine/ecs/components/velocity.hpp"
#include "../engine/ecs/components/label.hpp"
#include <SDL2/SDL.h>
#include <cmath>

InputSystem::InputSystem(float speed)
: controlled(INVALID_ENTITY), speed(speed)
{
    SDL_StartTextInput();
}

InputSystem::~InputSystem()
{
    SDL_StopTextInput();
}

void InputSystem::setControlled(Entity e) {
    controlled = e;
}

void InputSystem::update(Registry& reg, SDL_Event& e) {
    if (controlled == INVALID_ENTITY) return;

    const Uint8* ks = SDL_GetKeyboardState(NULL);

    if (reg.hasComponent<Velocity>(controlled)) {
        float vx = 0.f, vy = 0.f;
        if (ks[SDL_SCANCODE_UP] || ks[SDL_SCANCODE_W])    vy -= 1.f;
        if (ks[SDL_SCANCODE_DOWN] || ks[SDL_SCANCODE_S])  vy += 1.f;
        if (ks[SDL_SCANCODE_LEFT] || ks[SDL_SCANCODE_A])  vx -= 1.f;
        if (ks[SDL_SCANCODE_RIGHT] || ks[SDL_SCANCODE_D]) vx += 1.f;

        if (vx != 0.f || vy != 0.f) {
            float inv = 1.0f / std::sqrt(vx*vx + vy*vy);
            vx = vx * inv * speed;
            vy = vy * inv * speed;
        } else {
            vx = 0.f; vy = 0.f;
        }
        auto &vel = reg.getComponent<Velocity>(controlled);
        vel.vx = vx;
        vel.vy = vy;
    } else if (reg.hasComponent<Label>(controlled)) {
        if (e.type != 0) {
            if (e.type == SDL_TEXTINPUT) {
                auto &lbl = reg.getComponent<Label>(controlled);
                lbl.text.append(e.text.text);
            } else if (e.type == SDL_KEYDOWN) {
                auto &lbl = reg.getComponent<Label>(controlled);
                if (e.key.keysym.sym == SDLK_BACKSPACE && !lbl.text.empty()) {
                    lbl.text.pop_back();
                }
            }
        }
    }
}
