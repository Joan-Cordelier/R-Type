/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** CreateRoomMenu - UI for configuring room settings before creation
*/

#include "CreateRoomMenu.hpp"
#include <iostream>

CreateRoomMenu::CreateRoomMenu(Registry &reg) : _reg(reg) {
}

void CreateRoomMenu::init() {
    const float screenCenterX = 540.f;
    const float startY = 150.f;
    const float rowHeight = 70.f;
    
    // Layout constants - centered around screen
    const float labelX = screenCenterX - 200.f;     // Left-aligned labels
    const float leftBtnX = screenCenterX + 30.f;    // Left arrow button
    const float rightBtnX = screenCenterX + 230.f;  // Right arrow button
    const float valueX = (leftBtnX + 40.f + rightBtnX) / 2.f;  // Center of gap between buttons
    const int btnSize = 40;
    const float arrowOffset = 13.f;                 // Center arrow text in button

    // Title - centered
    _titleLabel = _reg.createEntity();
    _reg.addComponent<Position>(_titleLabel, screenCenterX - 100.f, 70.f);
    _reg.addComponent<Label>(_titleLabel, std::string("Create Room"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font"), Color(255, 255, 255), 200, false);
    _allEntities.push_back(_titleLabel);

    // ===== Max Players Row =====
    float rowY = startY;

    _maxPlayersLabel = _reg.createEntity();
    _reg.addComponent<Position>(_maxPlayersLabel, labelX, rowY);
    _reg.addComponent<Label>(_maxPlayersLabel, std::string("Max Players:"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(200, 200, 200), 200, false);
    _allEntities.push_back(_maxPlayersLabel);

    _maxPlayersLeftBtn = _reg.createEntity();
    _reg.addComponent<Position>(_maxPlayersLeftBtn, leftBtnX, rowY - 8.f);
    _reg.addComponent<Sprite>(_maxPlayersLeftBtn, std::string("textures/button/square_button.png"),
                              std::string("square_button"), btnSize, btnSize, 200, 0.f, 0.f, false);
    _reg.addComponent<Button>(_maxPlayersLeftBtn, std::string("create_room_players_left"), 200, false);
    _allEntities.push_back(_maxPlayersLeftBtn);

    _maxPlayersValueLabel = _reg.createEntity();
    _reg.addComponent<Position>(_maxPlayersValueLabel, valueX - 5.f, rowY);
    _reg.addComponent<Label>(_maxPlayersValueLabel, std::string("4"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 255, 100), 201, false);
    _allEntities.push_back(_maxPlayersValueLabel);

    _maxPlayersRightBtn = _reg.createEntity();
    _reg.addComponent<Position>(_maxPlayersRightBtn, rightBtnX, rowY - 8.f);
    _reg.addComponent<Sprite>(_maxPlayersRightBtn, std::string("textures/button/square_button.png"),
                              std::string("square_button"), btnSize, btnSize, 200, 0.f, 0.f, false);
    _reg.addComponent<Button>(_maxPlayersRightBtn, std::string("create_room_players_right"), 200, false);
    _allEntities.push_back(_maxPlayersRightBtn);

    Entity leftArrow1 = _reg.createEntity();
    _reg.addComponent<Position>(leftArrow1, leftBtnX + arrowOffset, rowY - 2.f);
    _reg.addComponent<Label>(leftArrow1, std::string("<"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 255, 255), 202, false);
    _allEntities.push_back(leftArrow1);

    Entity rightArrow1 = _reg.createEntity();
    _reg.addComponent<Position>(rightArrow1, rightBtnX + arrowOffset, rowY - 2.f);
    _reg.addComponent<Label>(rightArrow1, std::string(">"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 255, 255), 202, false);
    _allEntities.push_back(rightArrow1);

    // ===== Game Mode Row =====
    rowY += rowHeight;

    _gameModeLabel = _reg.createEntity();
    _reg.addComponent<Position>(_gameModeLabel, labelX, rowY);
    _reg.addComponent<Label>(_gameModeLabel, std::string("Game Mode:"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(200, 200, 200), 200, false);
    _allEntities.push_back(_gameModeLabel);

    _gameModeLeftBtn = _reg.createEntity();
    _reg.addComponent<Position>(_gameModeLeftBtn, leftBtnX, rowY - 8.f);
    _reg.addComponent<Sprite>(_gameModeLeftBtn, std::string("textures/button/square_button.png"),
                              std::string("square_button"), btnSize, btnSize, 200, 0.f, 0.f, false);
    _reg.addComponent<Button>(_gameModeLeftBtn, std::string("create_room_mode_left"), 200, false);
    _allEntities.push_back(_gameModeLeftBtn);

    _gameModeValueLabel = _reg.createEntity();
    _reg.addComponent<Position>(_gameModeValueLabel, valueX - 35.f, rowY);  // Offset for longer text
    _reg.addComponent<Label>(_gameModeValueLabel, std::string("Classic"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 255, 100), 201, false);
    _allEntities.push_back(_gameModeValueLabel);

    _gameModeRightBtn = _reg.createEntity();
    _reg.addComponent<Position>(_gameModeRightBtn, rightBtnX, rowY - 8.f);
    _reg.addComponent<Sprite>(_gameModeRightBtn, std::string("textures/button/square_button.png"),
                              std::string("square_button"), btnSize, btnSize, 200, 0.f, 0.f, false);
    _reg.addComponent<Button>(_gameModeRightBtn, std::string("create_room_mode_right"), 200, false);
    _allEntities.push_back(_gameModeRightBtn);

    Entity leftArrow2 = _reg.createEntity();
    _reg.addComponent<Position>(leftArrow2, leftBtnX + arrowOffset, rowY - 2.f);
    _reg.addComponent<Label>(leftArrow2, std::string("<"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 255, 255), 202, false);
    _allEntities.push_back(leftArrow2);

    Entity rightArrow2 = _reg.createEntity();
    _reg.addComponent<Position>(rightArrow2, rightBtnX + arrowOffset, rowY - 2.f);
    _reg.addComponent<Label>(rightArrow2, std::string(">"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 255, 255), 202, false);
    _allEntities.push_back(rightArrow2);

    // ===== Difficulty Row =====
    rowY += rowHeight;

    _difficultyLabel = _reg.createEntity();
    _reg.addComponent<Position>(_difficultyLabel, labelX, rowY);
    _reg.addComponent<Label>(_difficultyLabel, std::string("Difficulty:"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(200, 200, 200), 200, false);
    _allEntities.push_back(_difficultyLabel);

    _difficultyLeftBtn = _reg.createEntity();
    _reg.addComponent<Position>(_difficultyLeftBtn, leftBtnX, rowY - 8.f);
    _reg.addComponent<Sprite>(_difficultyLeftBtn, std::string("textures/button/square_button.png"),
                              std::string("square_button"), btnSize, btnSize, 200, 0.f, 0.f, false);
    _reg.addComponent<Button>(_difficultyLeftBtn, std::string("create_room_diff_left"), 200, false);
    _allEntities.push_back(_difficultyLeftBtn);

    _difficultyValueLabel = _reg.createEntity();
    _reg.addComponent<Position>(_difficultyValueLabel, valueX - 35.f, rowY);
    _reg.addComponent<Label>(_difficultyValueLabel, std::string("Normal"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 255, 100), 201, false);
    _allEntities.push_back(_difficultyValueLabel);

    _difficultyRightBtn = _reg.createEntity();
    _reg.addComponent<Position>(_difficultyRightBtn, rightBtnX, rowY - 8.f);
    _reg.addComponent<Sprite>(_difficultyRightBtn, std::string("textures/button/square_button.png"),
                              std::string("square_button"), btnSize, btnSize, 200, 0.f, 0.f, false);
    _reg.addComponent<Button>(_difficultyRightBtn, std::string("create_room_diff_right"), 200, false);
    _allEntities.push_back(_difficultyRightBtn);

    Entity leftArrow3 = _reg.createEntity();
    _reg.addComponent<Position>(leftArrow3, leftBtnX + arrowOffset, rowY - 2.f);
    _reg.addComponent<Label>(leftArrow3, std::string("<"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 255, 255), 202, false);
    _allEntities.push_back(leftArrow3);

    Entity rightArrow3 = _reg.createEntity();
    _reg.addComponent<Position>(rightArrow3, rightBtnX + arrowOffset, rowY - 2.f);
    _reg.addComponent<Label>(rightArrow3, std::string(">"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 255, 255), 202, false);
    _allEntities.push_back(rightArrow3);

    // ===== Action Buttons (centered) =====
    rowY += rowHeight + 50.f;
    const int actionBtnWidth = 140;
    const int actionBtnHeight = 60;
    const float btnGap = 30.f;

    // Confirm button - left of center
    float confirmBtnX = screenCenterX - actionBtnWidth - btnGap/2;
    _confirmButton = _reg.createEntity();
    _reg.addComponent<Position>(_confirmButton, confirmBtnX, rowY);
    _reg.addComponent<Sprite>(_confirmButton, std::string("textures/button/square_button.png"),
                              std::string("square_button"), actionBtnWidth, actionBtnHeight, 200, 0.f, 0.f, false);
    _reg.addComponent<Button>(_confirmButton, std::string("create_room_confirm"), 200, false);
    _allEntities.push_back(_confirmButton);

    _confirmLabel = _reg.createEntity();
    _reg.addComponent<Position>(_confirmLabel, confirmBtnX + actionBtnWidth/2 - 30.f, rowY + 18.f);
    _reg.addComponent<Label>(_confirmLabel, std::string("Create"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(100, 255, 100), 201, false);
    _allEntities.push_back(_confirmLabel);

    // Cancel button - right of center
    float cancelBtnX = screenCenterX + btnGap/2;
    _cancelButton = _reg.createEntity();
    _reg.addComponent<Position>(_cancelButton, cancelBtnX, rowY);
    _reg.addComponent<Sprite>(_cancelButton, std::string("textures/button/square_button.png"),
                              std::string("square_button"), actionBtnWidth, actionBtnHeight, 200, 0.f, 0.f, false);
    _reg.addComponent<Button>(_cancelButton, std::string("create_room_cancel"), 200, false);
    _allEntities.push_back(_cancelButton);

    _cancelLabel = _reg.createEntity();
    _reg.addComponent<Position>(_cancelLabel, cancelBtnX + actionBtnWidth/2 - 30.f, rowY + 18.f);  // Center "Cancel" in button
    _reg.addComponent<Label>(_cancelLabel, std::string("Cancel"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 100, 100), 201, false);
    _allEntities.push_back(_cancelLabel);

    std::cout << "[CreateRoomMenu] Initialized with " << _allEntities.size() << " entities" << std::endl;
}

void CreateRoomMenu::setup(ButtonSystem &buttonsys) {
    // Max Players arrows
    buttonsys.registerHandler("create_room_players_left", [this](Registry &r, Entity e) {
        if (_visible) cycleMaxPlayers(-1);
    });
    buttonsys.registerHandler("create_room_players_right", [this](Registry &r, Entity e) {
        if (_visible) cycleMaxPlayers(1);
    });

    // Game Mode arrows
    buttonsys.registerHandler("create_room_mode_left", [this](Registry &r, Entity e) {
        if (_visible) cycleGameMode(-1);
    });
    buttonsys.registerHandler("create_room_mode_right", [this](Registry &r, Entity e) {
        if (_visible) cycleGameMode(1);
    });

    // Difficulty arrows
    buttonsys.registerHandler("create_room_diff_left", [this](Registry &r, Entity e) {
        if (_visible) cycleDifficulty(-1);
    });
    buttonsys.registerHandler("create_room_diff_right", [this](Registry &r, Entity e) {
        if (_visible) cycleDifficulty(1);
    });

    // Confirm/Cancel
    buttonsys.registerHandler("create_room_confirm", [this](Registry &r, Entity e) {
        if (_visible && _onConfirm) {
            std::cout << "[CreateRoomMenu] Confirm clicked - MaxPlayers: " << (int)_config.maxPlayers
                      << ", Mode: " << _config.getGameModeStr()
                      << ", Difficulty: " << _config.getDifficultyStr() << std::endl;
            _onConfirm(_config);
        }
    });

    buttonsys.registerHandler("create_room_cancel", [this](Registry &r, Entity e) {
        if (_visible && _onCancel) {
            std::cout << "[CreateRoomMenu] Cancel clicked" << std::endl;
            _onCancel();
        }
    });
}

void CreateRoomMenu::show() {
    _visible = true;
    // Reset to defaults
    _config.maxPlayers = 4;
    _config.gameMode = GameMode::CLASSIC;
    _config.difficulty = Difficulty::NORMAL;
    updateLabels();

    for (Entity e : _allEntities) {
        if (_reg.hasComponent<Sprite>(e)) {
            _reg.getComponent<Sprite>(e).visible = true;
        }
        if (_reg.hasComponent<Label>(e)) {
            _reg.getComponent<Label>(e).visible = true;
        }
        if (_reg.hasComponent<Button>(e)) {
            _reg.getComponent<Button>(e).enabled = true;
        }
    }
    std::cout << "[CreateRoomMenu] Shown" << std::endl;
}

void CreateRoomMenu::hide() {
    _visible = false;
    for (Entity e : _allEntities) {
        if (_reg.hasComponent<Sprite>(e)) {
            _reg.getComponent<Sprite>(e).visible = false;
        }
        if (_reg.hasComponent<Label>(e)) {
            _reg.getComponent<Label>(e).visible = false;
        }
        if (_reg.hasComponent<Button>(e)) {
            _reg.getComponent<Button>(e).enabled = false;
        }
    }
    std::cout << "[CreateRoomMenu] Hidden" << std::endl;
}

void CreateRoomMenu::updateLabels() {
    if (_reg.hasComponent<Label>(_maxPlayersValueLabel)) {
        _reg.getComponent<Label>(_maxPlayersValueLabel).text = std::to_string(_config.maxPlayers);
    }
    if (_reg.hasComponent<Label>(_gameModeValueLabel)) {
        _reg.getComponent<Label>(_gameModeValueLabel).text = _config.getGameModeStr();
    }
    if (_reg.hasComponent<Label>(_difficultyValueLabel)) {
        _reg.getComponent<Label>(_difficultyValueLabel).text = _config.getDifficultyStr();
    }
}

void CreateRoomMenu::cycleMaxPlayers(int delta) {
    int current = _config.maxPlayers;
    current += delta;
    if (current < 1) current = 4;
    if (current > 4) current = 1;
    _config.maxPlayers = static_cast<uint8_t>(current);
    updateLabels();
    std::cout << "[CreateRoomMenu] Max Players: " << (int)_config.maxPlayers << std::endl;
}

void CreateRoomMenu::cycleGameMode(int delta) {
    int current = static_cast<int>(_config.gameMode);
    current += delta;
    if (current < 0) current = 3;  // Wrap to SPEEDY
    if (current > 3) current = 0;  // Wrap to CLASSIC
    _config.gameMode = static_cast<GameMode>(current);
    updateLabels();
    std::cout << "[CreateRoomMenu] Game Mode: " << _config.getGameModeStr() << std::endl;
}

void CreateRoomMenu::cycleDifficulty(int delta) {
    int current = static_cast<int>(_config.difficulty);
    current += delta;
    if (current < 0) current = 3;  // Wrap to NIGHTMARE
    if (current > 3) current = 0;  // Wrap to EASY
    _config.difficulty = static_cast<Difficulty>(current);
    updateLabels();
    std::cout << "[CreateRoomMenu] Difficulty: " << _config.getDifficultyStr() << std::endl;
}
