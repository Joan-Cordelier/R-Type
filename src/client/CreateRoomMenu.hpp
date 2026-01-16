/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** CreateRoomMenu - UI for configuring room settings before creation
*/

#ifndef CREATERROOMMENU_HPP_
#define CREATERROOMMENU_HPP_

#include "../common/Data/RoomConfig.hpp"
#include "../common/ecs/components/button.hpp"
#include "../common/ecs/components/label.hpp"
#include "../common/ecs/components/position.hpp"
#include "../common/ecs/components/sprite.hpp"
#include "../common/ecs/registry.hpp"
#include "../common/ecs/systems/button_system.hpp"
#include <functional>
#include <vector>

class CreateRoomMenu {
public:
    CreateRoomMenu(Registry &reg);

    void init();
    void setup(ButtonSystem &buttonsys);

    void show();
    void hide();
    bool isVisible() const { return _visible; }

    // Callbacks
    void setOnConfirm(std::function<void(const RoomConfig &)> callback) { _onConfirm = callback; }
    void setOnCancel(std::function<void()> callback) { _onCancel = callback; }

    // Get current configuration
    const RoomConfig &getConfig() const { return _config; }

private:
    void updateLabels();
    void cycleMaxPlayers(int delta);
    void cycleGameMode(int delta);
    void cycleDifficulty(int delta);

    Registry &_reg;
    bool _visible = false;
    RoomConfig _config;

    // UI Entities
    Entity _titleLabel;
    Entity _backgroundPanel;

    // Max Players row
    Entity _maxPlayersLabel;
    Entity _maxPlayersValueLabel;
    Entity _maxPlayersLeftBtn;
    Entity _maxPlayersRightBtn;

    // Game Mode row
    Entity _gameModeLabel;
    Entity _gameModeValueLabel;
    Entity _gameModeLeftBtn;
    Entity _gameModeRightBtn;

    // Difficulty row
    Entity _difficultyLabel;
    Entity _difficultyValueLabel;
    Entity _difficultyLeftBtn;
    Entity _difficultyRightBtn;

    // Action buttons
    Entity _confirmButton;
    Entity _confirmLabel;
    Entity _cancelButton;
    Entity _cancelLabel;

    // All entities for easy show/hide
    std::vector<Entity> _allEntities;

    // Callbacks
    std::function<void(const RoomConfig &)> _onConfirm;
    std::function<void()> _onCancel;
};

#endif /* !CREATERROOMMENU_HPP_ */
