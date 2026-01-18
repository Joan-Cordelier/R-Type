/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PauseMenu - Overlay menu shown when player pauses the game
*/

#include "PauseMenu.hpp"
#include <iostream>

PauseMenu::PauseMenu(Registry &reg) : _reg(reg) {
}

void PauseMenu::init() {
    // Create semi-transparent background panel (centered)
    _backgroundPanel = _reg.createEntity();
    _reg.addComponent<Position>(_backgroundPanel, 290.f, 15.f);
    _reg.addComponent<Sprite>(_backgroundPanel, std::string("textures/button/square_button.png"),
                              std::string("square_button"), 500, 400, 90, 0.f, -150.f, false);

    // Create "PAUSED" title
    _titleLabel = _reg.createEntity();
    _reg.addComponent<Position>(_titleLabel, 460.f, 45.f);
    _reg.addComponent<Label>(_titleLabel, std::string("PAUSED"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font"), Color(255, 255, 255), 95, false);

    // Create Resume button
    _resumeButton = _reg.createEntity();
    _reg.addComponent<Position>(_resumeButton, 390.f, 270.f);
    _reg.addComponent<Sprite>(_resumeButton, std::string("textures/button/square_button.png"),
                              std::string("square_button"), 300, 60, 100, 0.f, 0.f, false);
    _reg.addComponent<Button>(_resumeButton, std::string("pause_resume"), 300, false);

    _resumeButtonLabel = _reg.createEntity();
    _reg.addComponent<Position>(_resumeButtonLabel, 490.f, 285.f);
    _reg.addComponent<Label>(_resumeButtonLabel, std::string("Resume"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 255, 255), 101, false);

    // Create Settings button
    _settingsButton = _reg.createEntity();
    _reg.addComponent<Position>(_settingsButton, 390.f, 350.f);
    _reg.addComponent<Sprite>(_settingsButton, std::string("textures/button/square_button.png"),
                              std::string("square_button"), 300, 60, 100, 0.f, 0.f, false);
    _reg.addComponent<Button>(_settingsButton, std::string("pause_settings"), 100, false);

    _settingsButtonLabel = _reg.createEntity();
    _reg.addComponent<Position>(_settingsButtonLabel, 485.f, 365.f);
    _reg.addComponent<Label>(_settingsButtonLabel, std::string("Settings"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 255, 255), 101, false);

    // Create Exit Party button
    _exitPartyButton = _reg.createEntity();
    _reg.addComponent<Position>(_exitPartyButton, 390.f, 430.f);
    _reg.addComponent<Sprite>(_exitPartyButton, std::string("textures/button/square_button.png"),
                              std::string("square_button"), 300, 60, 100, 0.f, 0.f, false);
    _reg.addComponent<Button>(_exitPartyButton, std::string("pause_exit_party"), 100, false);

    _exitPartyButtonLabel = _reg.createEntity();
    _reg.addComponent<Position>(_exitPartyButtonLabel, 475.f, 445.f);
    _reg.addComponent<Label>(_exitPartyButtonLabel, std::string("Exit Party"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 255, 255), 101, false);

    std::cout << "[PauseMenu] Initialized" << std::endl;
}

void PauseMenu::setup(ButtonSystem &buttonsys) {
    buttonsys.registerHandler("pause_resume", [this]([[maybe_unused]] Registry &r, [[maybe_unused]] Entity e) {
        if (_onResume && _visible) {
            std::cout << "[PauseMenu] Resume clicked" << std::endl;
            _onResume();
        }
    });

    buttonsys.registerHandler("pause_settings", [this]([[maybe_unused]] Registry &r, [[maybe_unused]] Entity e) {
        if (_onSettings && _visible) {
            std::cout << "[PauseMenu] Settings clicked" << std::endl;
            _onSettings();
        }
    });

    buttonsys.registerHandler("pause_exit_party", [this]([[maybe_unused]] Registry &r, [[maybe_unused]] Entity e) {
        if (_onExitParty && _visible) {
            std::cout << "[PauseMenu] Exit Party clicked" << std::endl;
            _onExitParty();
        }
    });
}

void PauseMenu::show() {
    _visible = true;
    std::cout << "[PauseMenu] Showing pause menu" << std::endl;

    // Show background
    if (_reg.hasComponent<Sprite>(_backgroundPanel)) {
        _reg.getComponent<Sprite>(_backgroundPanel).visible = true;
    }

    // Show title
    if (_reg.hasComponent<Label>(_titleLabel)) {
        _reg.getComponent<Label>(_titleLabel).visible = true;
    }

    // Show Resume button
    if (_reg.hasComponent<Sprite>(_resumeButton)) {
        _reg.getComponent<Sprite>(_resumeButton).visible = true;
    }
    if (_reg.hasComponent<Button>(_resumeButton)) {
        _reg.getComponent<Button>(_resumeButton).enabled = true;
    }
    if (_reg.hasComponent<Label>(_resumeButtonLabel)) {
        _reg.getComponent<Label>(_resumeButtonLabel).visible = true;
    }

    // Show Settings button
    if (_reg.hasComponent<Sprite>(_settingsButton)) {
        _reg.getComponent<Sprite>(_settingsButton).visible = true;
    }
    if (_reg.hasComponent<Button>(_settingsButton)) {
        _reg.getComponent<Button>(_settingsButton).enabled = true;
    }
    if (_reg.hasComponent<Label>(_settingsButtonLabel)) {
        _reg.getComponent<Label>(_settingsButtonLabel).visible = true;
    }

    // Show Exit Party button
    if (_reg.hasComponent<Sprite>(_exitPartyButton)) {
        _reg.getComponent<Sprite>(_exitPartyButton).visible = true;
    }
    if (_reg.hasComponent<Button>(_exitPartyButton)) {
        _reg.getComponent<Button>(_exitPartyButton).enabled = true;
    }
    if (_reg.hasComponent<Label>(_exitPartyButtonLabel)) {
        _reg.getComponent<Label>(_exitPartyButtonLabel).visible = true;
    }
}

void PauseMenu::hide() {
    _visible = false;
    std::cout << "[PauseMenu] Hiding pause menu" << std::endl;

    // Hide background
    if (_reg.hasComponent<Sprite>(_backgroundPanel)) {
        _reg.getComponent<Sprite>(_backgroundPanel).visible = false;
    }

    // Hide title
    if (_reg.hasComponent<Label>(_titleLabel)) {
        _reg.getComponent<Label>(_titleLabel).visible = false;
    }

    // Hide Resume button
    if (_reg.hasComponent<Sprite>(_resumeButton)) {
        _reg.getComponent<Sprite>(_resumeButton).visible = false;
    }
    if (_reg.hasComponent<Button>(_resumeButton)) {
        _reg.getComponent<Button>(_resumeButton).enabled = false;
    }
    if (_reg.hasComponent<Label>(_resumeButtonLabel)) {
        _reg.getComponent<Label>(_resumeButtonLabel).visible = false;
    }

    // Hide Settings button
    if (_reg.hasComponent<Sprite>(_settingsButton)) {
        _reg.getComponent<Sprite>(_settingsButton).visible = false;
    }
    if (_reg.hasComponent<Button>(_settingsButton)) {
        _reg.getComponent<Button>(_settingsButton).enabled = false;
    }
    if (_reg.hasComponent<Label>(_settingsButtonLabel)) {
        _reg.getComponent<Label>(_settingsButtonLabel).visible = false;
    }

    // Hide Exit Party button
    if (_reg.hasComponent<Sprite>(_exitPartyButton)) {
        _reg.getComponent<Sprite>(_exitPartyButton).visible = false;
    }
    if (_reg.hasComponent<Button>(_exitPartyButton)) {
        _reg.getComponent<Button>(_exitPartyButton).enabled = false;
    }
    if (_reg.hasComponent<Label>(_exitPartyButtonLabel)) {
        _reg.getComponent<Label>(_exitPartyButtonLabel).visible = false;
    }
}

void PauseMenu::toggle() {
    if (_visible) {
        hide();
    } else {
        show();
    }
}
