#include "KeybindsManager.hpp"

#include <iostream>

KeybindsManager::KeybindsManager()
{

}

void KeybindsManager::assignNewKeybind(KeybindAction keybind, SDL_Scancode newVal)
{
    if (newVal < SDL_SCANCODE_A || newVal > SDL_SCANCODE_Z) { // only assign if key is A-Z
        if (newVal != SDL_SCANCODE_SPACE) { // or space
            return;
        }
    }

    switch (keybind) {
        case UpKey:
            upKey = newVal;
            break;
        case DownKey:
            downKey = newVal;
            break;
        case RightKey:
            rightKey = newVal;
            break;
        case LeftKey:
            leftKey = newVal;
            break;
        case ShootKey:
        std::cout << "Rebinding ShootKey to scancode " << newVal << std::endl;
            shootKey = newVal;
            break;
        default:
            break;
    }

    isWaitingForKeybind = false;
    currentKeybindAction = KeybindAction::None;
    
    // Notify callback if set
    if (onKeybindChanged) {
        onKeybindChanged(keybind, newVal);
    }
}

void KeybindsManager::setOnKeybindChangedCallback(std::function<void(KeybindAction, SDL_Scancode)> callback)
{
    onKeybindChanged = callback;
}

void KeybindsManager::initiateKeybindChange(KeybindAction action)
{
    std::cout << "Press a key to rebind action " << action << std::endl;
    isWaitingForKeybind = true;
    currentKeybindAction = action;
}