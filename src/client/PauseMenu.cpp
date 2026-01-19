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
    // Create "PAUSED" title
    _titleLabel = _reg.createEntity();
    _reg.addComponent<Position>(_titleLabel, 460.f, 190.f);
    _reg.addComponent<Label>(_titleLabel, std::string("PAUSED"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font"), Color(255, 215, 0), 95, false);

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

    // Create Exit button (quit game)
    _exitButton = _reg.createEntity();
    _reg.addComponent<Position>(_exitButton, 390.f, 510.f);
    _reg.addComponent<Sprite>(_exitButton, std::string("textures/button/square_button.png"),
                              std::string("square_button"), 300, 60, 100, 0.f, 0.f, false);
    _reg.addComponent<Button>(_exitButton, std::string("pause_exit"), 100, false);

    _exitButtonLabel = _reg.createEntity();
    _reg.addComponent<Position>(_exitButtonLabel, 505.f, 525.f);
    _reg.addComponent<Label>(_exitButtonLabel, std::string("Exit"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 100, 100), 101, false);

    std::cout << "[PauseMenu] Initialized" << std::endl;
}

void PauseMenu::setup(ButtonSystem &buttonsys) {
    buttonsys.registerHandler("pause_resume",
                              [this]([[maybe_unused]] Registry &r, [[maybe_unused]] Entity e) {
                                  if (_onResume && _visible) {
                                      std::cout << "[PauseMenu] Resume clicked" << std::endl;
                                      _onResume();
                                  }
                              });

    buttonsys.registerHandler("pause_settings",
                              [this]([[maybe_unused]] Registry &r, [[maybe_unused]] Entity e) {
                                  if (_onSettings && _visible) {
                                      std::cout << "[PauseMenu] Settings clicked" << std::endl;
                                      _onSettings();
                                  }
                              });

    buttonsys.registerHandler("pause_exit_party",
                              [this]([[maybe_unused]] Registry &r, [[maybe_unused]] Entity e) {
                                  if (_onExitParty && _visible) {
                                      std::cout << "[PauseMenu] Exit Party clicked" << std::endl;
                                      _onExitParty();
                                  }
                              });

    buttonsys.registerHandler(
        "pause_exit", [this]([[maybe_unused]] Registry &r, [[maybe_unused]] Entity e) {
            if (_onExit && _visible) {
                std::cout << "[PauseMenu] Exit clicked (quit game)" << std::endl;
                _onExit();
            }
        });
}

void PauseMenu::show() {
    _visible = true;
    std::cout << "[PauseMenu] Showing pause menu" << std::endl;

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

void PauseMenu::hide() {
    _visible = false;
    std::cout << "[PauseMenu] Hiding pause menu" << std::endl;

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

void PauseMenu::toggle() {
    if (_visible) {
        hide();
    } else {
        show();
    }
}
