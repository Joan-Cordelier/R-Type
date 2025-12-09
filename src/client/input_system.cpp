/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ArrowInputSystem
*/

#include "../common/ecs/registry.hpp"
#include "../common/ecs/components/position.hpp"
#include "../common/ecs/components/spritesheet.hpp"
#include "../common/ecs/components/stats.hpp"

#include "input_system.hpp"
#include "../common/ecs/components/velocity.hpp"
#include "../common/ecs/components/label.hpp"
#include <SDL2/SDL.h>
#include <cmath>

InputSystem::InputSystem() {
    last_x = 0;
    last_y = 0;
}

InputSystem::~InputSystem() {}

void InputSystem::setControlled(Entity e) {
    controlled = e;
}

void InputSystem::update(Registry& reg, SDL_Event& e, NetworkManager& networkManager) {
    if (controlled == INVALID_ENTITY) return;

    const Uint8* ks = SDL_GetKeyboardState(NULL);

    if (reg.hasComponent<Velocity>(controlled)) {
        float vx = 0.f, vy = 0.f;
        if (ks[SDL_SCANCODE_UP] || ks[SDL_SCANCODE_W])    vy -= 1.f;
        if (ks[SDL_SCANCODE_DOWN] || ks[SDL_SCANCODE_S])  vy += 1.f;
        if (ks[SDL_SCANCODE_LEFT] || ks[SDL_SCANCODE_A])  vx -= 1.f;
        if (ks[SDL_SCANCODE_RIGHT] || ks[SDL_SCANCODE_D]) vx += 1.f;

        if (last_x != vx || last_y != vy) {
            last_x = vx;
            last_y = vy;
            MessageFactory& factory = MessageFactory::getInstance();
            networkManager.sendUdp(factory.createMessage(OpCode::MOVE, factory.encodeMessageMovementPlayer(controlled, vx, vy)));
        }

        if (ks[SDL_SCANCODE_SPACE]) {
            if (!reg.hasComponent<Stats>(controlled))
                return;
            auto &stats = reg.getComponent<Stats>(controlled);
            if (!stats.canAttack())
                return;
            MessageFactory& factory = MessageFactory::getInstance();
            std::cout << "Sending SHOOT message for entity " << controlled << std::endl;
            MessageData payload = factory.encodeMessagePlayer(controlled);
            std::cout << "Encoded SHOOT payload size: " << payload.size() << std::endl;
            networkManager.sendUdp(factory.createMessage(OpCode::SHOOT, payload));
        }
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
