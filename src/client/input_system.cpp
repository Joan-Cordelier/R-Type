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
#include "../common/Data/EntityType.hpp"

#include "input_system.hpp"
#include "../common/ecs/components/velocity.hpp"
#include "../common/ecs/components/label.hpp"
#include "../common/ecs/components/weapon.hpp"
#include <SDL2/SDL.h>
#include <cmath>

InputSystem::InputSystem() {
    last_x = 0;
    last_y = 0;
    keybindsManager = nullptr;
}

InputSystem::~InputSystem() {}

void InputSystem::setControlled(Entity e, KeybindsManager& kbManager) {
    controlled = e;
    keybindsManager = &kbManager;
}

void InputSystem::update(Registry& reg, SDL_Event& e, NetworkManager& networkManager, WeaponSystem &weaponsys) {
    if (!keybindsManager) return;
    
    if (keybindsManager->isWaitingForKeybind) {
        if (e.type == SDL_KEYDOWN) {
            SDL_Scancode newKey = e.key.keysym.scancode;
            keybindsManager->assignNewKeybind(keybindsManager->currentKeybindAction, newKey);
        }
        return; // the label still catch the input even when we return here need to fix this later
    }
    
    if (controlled == INVALID_ENTITY) return;
    const Uint8* ks = SDL_GetKeyboardState(NULL);

    if (reg.hasComponent<Velocity>(controlled)) {
        float vx = 0.f, vy = 0.f;
        if (ks[SDL_SCANCODE_UP] || ks[keybindsManager->upKey])    vy -= 1.f;
        if (ks[SDL_SCANCODE_DOWN] || ks[keybindsManager->downKey])  vy += 1.f;
        if (ks[SDL_SCANCODE_LEFT] || ks[keybindsManager->leftKey])  vx -= 1.f;
        if (ks[SDL_SCANCODE_RIGHT] || ks[keybindsManager->rightKey]) vx += 1.f;

        if (last_x != vx || last_y != vy) {
            last_x = vx;
            last_y = vy;
            MessageFactory& factory = MessageFactory::getInstance();
            MessageData payload = factory.encodeMessageMoveInput(controlled, vx, vy);
            networkManager.sendUdp(factory.createMessage(OpCode::MOVE_INPUT, payload));
        }

        if (ks[keybindsManager->shootKey]) {
            if (!reg.hasComponent<Stats>(controlled))
                return;
            if (!reg.hasComponent<Weapon>(controlled))
                return;
            if (!weaponsys.canAttack(controlled))
                return;
            
            // Get current position
            float x = 0.f, y = 0.f;
            if (reg.hasComponent<Position>(controlled)) {
                Position& pos = reg.getComponent<Position>(controlled);
                x = pos.x;
                y = pos.y;
            }
            
            MessageFactory& factory = MessageFactory::getInstance();
            MessageData payload = factory.encodeMessageProjectile(0, controlled, std::string("player"), x, y, 1.0f);
            networkManager.sendUdp(factory.createMessage(OpCode::SHOOT, payload));
            weaponsys.fireWeapon(reg, controlled);
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
