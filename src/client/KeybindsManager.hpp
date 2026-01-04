#pragma once

#include <SDL2/SDL.h>

typedef enum {
    None,
    UpKey,
    DownKey,
    RightKey,
    LeftKey,
    ShootKey,
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
    SDL_Scancode shootKey = SDL_SCANCODE_SPACE;


    KeybindsManager();
    void assignNewKeybind(KeybindAction keybind, SDL_Scancode newVal);

};