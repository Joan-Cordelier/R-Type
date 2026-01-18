/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** DeathScreen - UI shown when player dies
*/

#include "DeathScreen.hpp"
#include <iostream>

DeathScreen::DeathScreen(Registry &reg) : _reg(reg) {
}

void DeathScreen::init() {
    // Create semi-transparent background panel
    _backgroundPanel = _reg.createEntity();
    _reg.addComponent<Position>(_backgroundPanel, 250.f, 150.f);
    _reg.addComponent<Sprite>(_backgroundPanel, std::string("textures/button/square_button.png"),
                              std::string("square_button"), 500, 350, 90, 0.f, 0.f, false);

    // Create "You Died!" title
    _titleLabel = _reg.createEntity();
    _reg.addComponent<Position>(_titleLabel, 390.f, 180.f);
    _reg.addComponent<Label>(_titleLabel, std::string("You Died!"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font"), Color(255, 50, 50), 95, false);

    // Create score label
    _scoreLabel = _reg.createEntity();
    _reg.addComponent<Position>(_scoreLabel, 350.f, 280.f);
    _reg.addComponent<Label>(_scoreLabel, std::string("Final Score: 0"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font"), Color(255, 215, 0), 95, false);

    // Create return to lobby button
    _returnButton = _reg.createEntity();
    _reg.addComponent<Position>(_returnButton, 350.f, 380.f);
    _reg.addComponent<Sprite>(_returnButton, std::string("textures/button/square_button.png"),
                              std::string("square_button"), 300, 60, 100, 0.f, 0.f, false);
    _reg.addComponent<Button>(_returnButton, std::string("death_return_lobby"), 100, false);

    // Return button label
    _returnButtonLabel = _reg.createEntity();
    _reg.addComponent<Position>(_returnButtonLabel, 410.f, 395.f);
    _reg.addComponent<Label>(_returnButtonLabel, std::string("Return to Lobby"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 255, 255), 101, false);

    std::cout << "[DeathScreen] Initialized" << std::endl;
}

void DeathScreen::setup(ButtonSystem &buttonsys) {
    buttonsys.registerHandler("death_return_lobby", [this]([[maybe_unused]] Registry &r, [[maybe_unused]] Entity e) {
        if (_onReturnToLobby && _visible) {
            std::cout << "[DeathScreen] Return to Lobby clicked" << std::endl;
            _onReturnToLobby();
        }
    });
}

void DeathScreen::show(uint32_t finalScore) {
    _visible = true;
    std::cout << "[DeathScreen] Showing death screen with score: " << finalScore << std::endl;

    // Update score label
    if (_reg.hasComponent<Label>(_scoreLabel)) {
        _reg.getComponent<Label>(_scoreLabel).text = "Final Score: " + std::to_string(finalScore);
    }

    // Show background
    if (_reg.hasComponent<Sprite>(_backgroundPanel)) {
        _reg.getComponent<Sprite>(_backgroundPanel).visible = true;
    }

    // Show title
    if (_reg.hasComponent<Label>(_titleLabel)) {
        _reg.getComponent<Label>(_titleLabel).visible = true;
    }

    // Show score
    if (_reg.hasComponent<Label>(_scoreLabel)) {
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
}

void DeathScreen::hide() {
    _visible = false;
    std::cout << "[DeathScreen] Hiding death screen" << std::endl;

    // Hide background
    if (_reg.hasComponent<Sprite>(_backgroundPanel)) {
        _reg.getComponent<Sprite>(_backgroundPanel).visible = false;
    }

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
}
