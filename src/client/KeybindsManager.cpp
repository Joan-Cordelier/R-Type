#include "KeybindsManager.hpp"

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
            shootKey = newVal;
            break;
        default:
            break;
    }
}