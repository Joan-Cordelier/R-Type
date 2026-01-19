/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** EndGameScreen - UI shown when game ends (win or lose)
*/

#ifndef ENDGAMESCREEN_HPP_
#define ENDGAMESCREEN_HPP_

#include "../common/ecs/ecs_components.hpp"
#include "../common/ecs/ecs_system.hpp"
#include "../common/ecs/registry.hpp"
#include <functional>
#include <string>

class EndGameScreen {
public:
    using ReturnToLobbyCallback = std::function<void()>;
    using ExitCallback = std::function<void()>;

    EndGameScreen(Registry &reg);
    ~EndGameScreen() = default;

    void init();
    void setup(ButtonSystem &buttonsys);
    void showDeath(uint32_t finalScore);   // Show "You Died!" in red
    void showVictory(uint32_t finalScore); // Show "YOU WIN!" in green
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
    void showScreen(uint32_t finalScore, const std::string &title, Color titleColor);

    Registry &_reg;
    bool _visible = false;

    // UI Entities
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

#endif /* !ENDGAMESCREEN_HPP_ */
