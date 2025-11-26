/*
** EPITECH PROJECT, 2025
** local
** File description:
** Window
*/

#include "Window.hpp"


Window::Window()
{
    int initresult = SDL_Init(SDL_INIT_EVERYTHING);
    if (initresult != 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        exit(84);
    }
    int result = SDL_CreateWindowAndRenderer(1080, 720, 0, &win, &renderer);

    if (result != 0)
        std::cerr << "Failed to create a window and renderer: " << SDL_GetError() << std::endl;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
}

Window::~Window()
{
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (win)
        SDL_DestroyWindow(win);
    SDL_Quit();
}

int Window::draw()
{
    SDL_RenderPresent(renderer);
    return 0;
}

PollEvent Window::pollEvent()
{
    if (SDL_PollEvent(&ev) == 0) {
        return {PollStatus::NONE};
    }
    
    PollEvent event;
    
    switch (ev.type) {
        case SDL_QUIT:
            event.type = PollStatus::QUIT;
            break;
            
        case SDL_KEYDOWN:
            event.type = PollStatus::KEYDOWN;
            event.key.key = sdlKeyToKeyCode(ev.key.keysym.sym);
            event.key.repeat = ev.key.repeat != 0;
            break;
            
        case SDL_KEYUP:
            event.type = PollStatus::KEYUP;
            event.key.key = sdlKeyToKeyCode(ev.key.keysym.sym);
            event.key.repeat = false;
            break;
            
        case SDL_MOUSEBUTTONDOWN:
            event.type = PollStatus::MOUSEBUTTONDOWN;
            event.mouseButton.button = sdlButtonToMouseButton(ev.button.button);
            event.mouseButton.x = ev.button.x;
            event.mouseButton.y = ev.button.y;
            event.mouseButton.clicks = ev.button.clicks;
            break;
            
        case SDL_MOUSEBUTTONUP:
            event.type = PollStatus::MOUSEBUTTONUP;
            event.mouseButton.button = sdlButtonToMouseButton(ev.button.button);
            event.mouseButton.x = ev.button.x;
            event.mouseButton.y = ev.button.y;
            break;
            
        case SDL_MOUSEMOTION:
            event.type = PollStatus::MOUSEMOTION;
            event.mouseMotion.x = ev.motion.x;
            event.mouseMotion.y = ev.motion.y;
            event.mouseMotion.xrel = ev.motion.xrel;
            event.mouseMotion.yrel = ev.motion.yrel;
            break;
            
        default:
            event.type = PollStatus::NONE;
            break;
    }
    
    return event;
}

void Window::clear()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void Window::clear(Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer);
}

KeyCode Window::sdlKeyToKeyCode(SDL_Keycode sdlKey) {
    switch (sdlKey) {
        case SDLK_a: return KeyCode::A;
        case SDLK_b: return KeyCode::B;
        case SDLK_w: return KeyCode::W;
        case SDLK_s: return KeyCode::S;
        case SDLK_d: return KeyCode::D;
        case SDLK_SPACE: return KeyCode::SPACE;
        case SDLK_RETURN: return KeyCode::ENTER;
        case SDLK_ESCAPE: return KeyCode::ESCAPE;
        case SDLK_LEFT: return KeyCode::LEFT;
        case SDLK_RIGHT: return KeyCode::RIGHT;
        case SDLK_UP: return KeyCode::UP;
        case SDLK_DOWN: return KeyCode::DOWN;
        // Add more mappings
        default: return KeyCode::UNKNOWN;
    }
}

MouseButton Window::sdlButtonToMouseButton(Uint8 sdlButton) {
    switch (sdlButton) {
        case SDL_BUTTON_LEFT: return MouseButton::LEFT;
        case SDL_BUTTON_MIDDLE: return MouseButton::MIDDLE;
        case SDL_BUTTON_RIGHT: return MouseButton::RIGHT;
        default: return MouseButton::UNKNOWN;
    }
}