#pragma once

#include <SDL2/SDL.h>
#include <functional>

typedef enum {
    None,
    UpKey,
    DownKey,
    RightKey,
    LeftKey,
} KeybindAction;

class KeybindsManager
{
private:

public:
    //keybinds
    SDL_Scancode upKey = SDL_SCANCODE_W;
    SDL_Scancode downKey = SDL_SCANCODE_S;
    SDL_Scancode rightKey = SDL_SCANCODE_D;
    SDL_Scancode leftKey = SDL_SCANCODE_A;

    bool isWaitingForKeybind = false;
    KeybindAction currentKeybindAction = KeybindAction::None;
    std::function<void(KeybindAction, SDL_Scancode)> onKeybindChanged;

    KeybindsManager();
    void initiateKeybindChange(KeybindAction action);
    void assignNewKeybind(KeybindAction keybind, SDL_Scancode newVal);
    void setOnKeybindChangedCallback(std::function<void(KeybindAction, SDL_Scancode)> callback);

};