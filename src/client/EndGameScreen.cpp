/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** EndGameScreen - UI shown when game ends (win or lose)
*/

#include "EndGameScreen.hpp"
#include <iostream>

EndGameScreen::EndGameScreen(Registry &reg) : _reg(reg) {
}

void EndGameScreen::init() {
    // Create title label (text and color set dynamically)
    _titleLabel = _reg.createEntity();
    _reg.addComponent<Position>(_titleLabel, 460.f, 180.f);
    _reg.addComponent<Label>(_titleLabel, std::string(""),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font"), Color(255, 255, 255), 95, false);

    // Create score label
    _scoreLabel = _reg.createEntity();
    _reg.addComponent<Position>(_scoreLabel, 400.f, 280.f);
    _reg.addComponent<Label>(_scoreLabel, std::string("Final Score: 0"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font"), Color(255, 215, 0), 95, false);

    // Create return to lobby button
    _returnButton = _reg.createEntity();
    _reg.addComponent<Position>(_returnButton, 390.f, 355.f);
    _reg.addComponent<Sprite>(_returnButton, std::string("textures/button/square_button.png"),
                              std::string("square_button"), 330, 160, 100, 0.f, -25.f, false);
    _reg.addComponent<Button>(_returnButton, std::string("endgame_return_lobby"), 100, false);

    // Return button label
    _returnButtonLabel = _reg.createEntity();
    _reg.addComponent<Position>(_returnButtonLabel, 460.f, 395.f);
    _reg.addComponent<Label>(_returnButtonLabel, std::string("Return to Lobby"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 255, 255), 101, false);

    // Create Exit button (quit game)
    _exitButton = _reg.createEntity();
    _reg.addComponent<Position>(_exitButton, 390.f, 475.f);
    _reg.addComponent<Sprite>(_exitButton, std::string("textures/button/square_button.png"),
                              std::string("square_button"), 330, 160, 100, 0.f, -25.f, false);
    _reg.addComponent<Button>(_exitButton, std::string("endgame_exit"), 100, false);

    _exitButtonLabel = _reg.createEntity();
    _reg.addComponent<Position>(_exitButtonLabel, 515.f, 515.f);
    _reg.addComponent<Label>(_exitButtonLabel, std::string("Exit"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 100, 100), 101, false);

    std::cout << "[EndGameScreen] Initialized" << std::endl;
}

void EndGameScreen::setup(ButtonSystem &buttonsys) {
    buttonsys.registerHandler(
        "endgame_return_lobby", [this]([[maybe_unused]] Registry &r, [[maybe_unused]] Entity e) {
            if (_onReturnToLobby && _visible) {
                std::cout << "[EndGameScreen] Return to Lobby clicked" << std::endl;
                _onReturnToLobby();
            }
        });

    buttonsys.registerHandler(
        "endgame_exit", [this]([[maybe_unused]] Registry &r, [[maybe_unused]] Entity e) {
            if (_onExit && _visible) {
                std::cout << "[EndGameScreen] Exit clicked (quit game)" << std::endl;
                _onExit();
            }
        });
}

void EndGameScreen::showDeath(uint32_t finalScore) {
    showScreen(finalScore, "You Died!", Color(255, 50, 50));
}

void EndGameScreen::showVictory(uint32_t finalScore) {
    showScreen(finalScore, "YOU WIN!", Color(50, 255, 50));
}

void EndGameScreen::showScreen(uint32_t finalScore, const std::string &title, Color titleColor) {
    _visible = true;
    std::cout << "[EndGameScreen] Showing: " << title << " with score: " << finalScore << std::endl;

    // Update title
    if (_reg.hasComponent<Label>(_titleLabel)) {
        auto &label = _reg.getComponent<Label>(_titleLabel);
        label.text = title;
        label.color = titleColor;
        label.visible = true;
    }

    // Update score label
    if (_reg.hasComponent<Label>(_scoreLabel)) {
        _reg.getComponent<Label>(_scoreLabel).text = "Final Score: " + std::to_string(finalScore);
        _reg.getComponent<Label>(_scoreLabel).visible = true;
    }

    // Show button
    if (_reg.hasComponent<Sprite>(_returnButton)) {
        _reg.getComponent<Sprite>(_returnButton).visible = true;
    }
    if (_reg.hasComponent<Button>(_returnButton)) {
        _reg.getComponent<Button>(_returnButton).enabled = true;
    }

    // Show button label
    if (_reg.hasComponent<Label>(_returnButtonLabel)) {
        _reg.getComponent<Label>(_returnButtonLabel).visible = true;
    }

    // Show Exit button
    if (_reg.hasComponent<Sprite>(_exitButton)) {
        _reg.getComponent<Sprite>(_exitButton).visible = true;
    }
    if (_reg.hasComponent<Button>(_exitButton)) {
        _reg.getComponent<Button>(_exitButton).enabled = true;
    }
    if (_reg.hasComponent<Label>(_exitButtonLabel)) {
        _reg.getComponent<Label>(_exitButtonLabel).visible = true;
    }
}

void EndGameScreen::hide() {
    _visible = false;
    std::cout << "[EndGameScreen] Hiding end game screen" << std::endl;

    // Hide title
    if (_reg.hasComponent<Label>(_titleLabel)) {
        _reg.getComponent<Label>(_titleLabel).visible = false;
    }

    // Hide score
    if (_reg.hasComponent<Label>(_scoreLabel)) {
        _reg.getComponent<Label>(_scoreLabel).visible = false;
    }

    // Hide button
    if (_reg.hasComponent<Sprite>(_returnButton)) {
        _reg.getComponent<Sprite>(_returnButton).visible = false;
    }
    if (_reg.hasComponent<Button>(_returnButton)) {
        _reg.getComponent<Button>(_returnButton).enabled = false;
    }

    // Hide button label
    if (_reg.hasComponent<Label>(_returnButtonLabel)) {
        _reg.getComponent<Label>(_returnButtonLabel).visible = false;
    }

    // Hide Exit button
    if (_reg.hasComponent<Sprite>(_exitButton)) {
        _reg.getComponent<Sprite>(_exitButton).visible = false;
    }
    if (_reg.hasComponent<Button>(_exitButton)) {
        _reg.getComponent<Button>(_exitButton).enabled = false;
    }
    if (_reg.hasComponent<Label>(_exitButtonLabel)) {
        _reg.getComponent<Label>(_exitButtonLabel).visible = false;
    }
}
