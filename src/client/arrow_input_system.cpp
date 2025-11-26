/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ArrowInputSystem
*/

#include "arrow_input_system.hpp"
#include <SDL2/SDL.h>
#include "../engine/ecs/components/velocity.hpp"
#include <cmath>

ArrowInputSystem::ArrowInputSystem(float speed)
: controlled(INVALID_ENTITY), speed(speed) {}

void ArrowInputSystem::setControlled(Entity e) {
    controlled = e;
}

void ArrowInputSystem::update(Registry& reg) {
    if (controlled == INVALID_ENTITY) return;

    SDL_PumpEvents();
    const Uint8* ks = SDL_GetKeyboardState(NULL);

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

    if (reg.hasComponent<Velocity>(controlled)) {
        auto &vel = reg.getComponent<Velocity>(controlled);
        vel.vx = vx;
        vel.vy = vy;
    } else {
        reg.addComponent<Velocity>(controlled, vx, vy);
    }
}
