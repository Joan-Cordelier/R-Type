/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PauseMenu - Overlay menu shown when player pauses the game
*/

#ifndef PAUSEMENU_HPP_
#define PAUSEMENU_HPP_

#include "../common/ecs/ecs_components.hpp"
#include "../common/ecs/ecs_system.hpp"
#include "../common/ecs/registry.hpp"
#include <functional>
#include <string>

class PauseMenu {
public:
    using ResumeCallback = std::function<void()>;
    using SettingsCallback = std::function<void()>;
    using ExitPartyCallback = std::function<void()>;
    using ExitCallback = std::function<void()>;

    PauseMenu(Registry &reg);
    ~PauseMenu() = default;

    void init();
    void setup(ButtonSystem &buttonsys);
    void show();
    void hide();
    void toggle();
    bool isVisible() const {
        return _visible;
    }

    void setResumeCallback(ResumeCallback callback) {
        _onResume = callback;
    }
    void setSettingsCallback(SettingsCallback callback) {
        _onSettings = callback;
    }
    void setExitPartyCallback(ExitPartyCallback callback) {
        _onExitParty = callback;
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
    Entity _resumeButton;
    Entity _resumeButtonLabel;
    Entity _settingsButton;
    Entity _settingsButtonLabel;
    Entity _exitPartyButton;
    Entity _exitPartyButtonLabel;
    Entity _exitButton;
    Entity _exitButtonLabel;

    // Callbacks
    ResumeCallback _onResume;
    SettingsCallback _onSettings;
    ExitPartyCallback _onExitParty;
    ExitCallback _onExit;
};

#endif /* !PAUSEMENU_HPP_ */
