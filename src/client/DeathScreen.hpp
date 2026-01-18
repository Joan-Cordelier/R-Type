/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** DeathScreen - UI shown when player dies
*/

#ifndef DEATHSCREEN_HPP_
#define DEATHSCREEN_HPP_

#include "../common/ecs/ecs_components.hpp"
#include "../common/ecs/ecs_system.hpp"
#include "../common/ecs/registry.hpp"
#include <functional>
#include <string>

class DeathScreen {
public:
    using ReturnToLobbyCallback = std::function<void()>;
    using ExitCallback = std::function<void()>;

    DeathScreen(Registry &reg);
    ~DeathScreen() = default;

    void init();
    void setup(ButtonSystem &buttonsys);
    void show(uint32_t finalScore);
    void hide();
    bool isVisible() const {
        return _visible;
    }

    void setReturnToLobbyCallback(ReturnToLobbyCallback callback) {
        _onReturnToLobby = callback;
    }
    void setExitCallback(ExitCallback callback) {
        _onExit = callback;
    }

private:
    Registry &_reg;
    bool _visible = false;

    // UI Entities
    Entity _backgroundPanel;
    Entity _titleLabel;
    Entity _scoreLabel;
    Entity _returnButton;
    Entity _returnButtonLabel;
    Entity _exitButton;
    Entity _exitButtonLabel;

    // Callbacks
    ReturnToLobbyCallback _onReturnToLobby;
    ExitCallback _onExit;
};

#endif /* !DEATHSCREEN_HPP_ */
