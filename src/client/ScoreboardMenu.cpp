/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ScoreboardMenu - Client scoreboard UI for displaying top scores
*/

#include "ScoreboardMenu.hpp"
#include <iostream>

ScoreboardMenu::ScoreboardMenu(Registry &reg) : _reg(reg) {
}

void ScoreboardMenu::init() {
    // Create semi-transparent background panel
    _backgroundPanel = _reg.createEntity();
    _reg.addComponent<Position>(_backgroundPanel, 200.f, 80.f);
    _reg.addComponent<Sprite>(_backgroundPanel, std::string("textures/button/square_button.png"),
                              std::string("square_button"), 600, 520, 90, 0.f, 0.f, false);

    // Create title label
    _titleLabel = _reg.createEntity();
    _reg.addComponent<Position>(_titleLabel, 360.f, 100.f);
    _reg.addComponent<Label>(_titleLabel, std::string("Scoreboard"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("scoreboard_title_font"), Color(255, 215, 0), 95, false);

    // Create back button
    _closeButton = _reg.createEntity();
    _reg.addComponent<Position>(_closeButton, 250.f, 520.f);
    _reg.addComponent<Sprite>(_closeButton, std::string("textures/button/square_button.png"),
                              std::string("square_button"), 150, 50, 100, 0.f, 0.f, false);
    _reg.addComponent<Button>(_closeButton, std::string("scoreboard_close"), 100, false);

    // Back button label
    Entity closeLabel = _reg.createEntity();
    _reg.addComponent<Position>(closeLabel, 300.f, 535.f);
    _reg.addComponent<Label>(closeLabel, std::string("Back"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("scoreboard_close_label"), Color(255, 255, 255), 101, false);
    _scoreEntities.push_back(closeLabel);

    // Create refresh button
    _refreshButton = _reg.createEntity();
    _reg.addComponent<Position>(_refreshButton, 600.f, 520.f);
    _reg.addComponent<Sprite>(_refreshButton, std::string("textures/button/square_button.png"),
                              std::string("square_button"), 150, 50, 100, 0.f, 0.f, false);
    _reg.addComponent<Button>(_refreshButton, std::string("scoreboard_refresh"), 100, false);

    // Refresh button label
    Entity refreshLabel = _reg.createEntity();
    _reg.addComponent<Position>(refreshLabel, 640.f, 535.f);
    _reg.addComponent<Label>(refreshLabel, std::string("Refresh"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("scoreboard_refresh_label"), Color(255, 255, 255), 101, false);
    _scoreEntities.push_back(refreshLabel);

    // Create header labels
    Entity rankHeader = _reg.createEntity();
    _reg.addComponent<Position>(rankHeader, 230.f, 170.f);
    _reg.addComponent<Label>(rankHeader, std::string("#"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("scoreboard_header_rank"), Color(200, 200, 200), 95, false);
    _scoreEntities.push_back(rankHeader);

    Entity nameHeader = _reg.createEntity();
    _reg.addComponent<Position>(nameHeader, 300.f, 170.f);
    _reg.addComponent<Label>(nameHeader, std::string("Player"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("scoreboard_header_name"), Color(200, 200, 200), 95, false);
    _scoreEntities.push_back(nameHeader);

    Entity scoreHeader = _reg.createEntity();
    _reg.addComponent<Position>(scoreHeader, 600.f, 170.f);
    _reg.addComponent<Label>(scoreHeader, std::string("Score"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("scoreboard_header_score"), Color(200, 200, 200), 95, false);
    _scoreEntities.push_back(scoreHeader);

    std::cout << "[ScoreboardMenu] Initialized" << std::endl;
}

void ScoreboardMenu::setup(ButtonSystem &buttonsys) {
    buttonsys.registerHandler("scoreboard_close", [this]([[maybe_unused]] Registry &r, [[maybe_unused]] Entity e) {
        if (_onClose && _visible) {
            std::cout << "[ScoreboardMenu] Close clicked" << std::endl;
            _onClose();
        }
    });

    buttonsys.registerHandler("scoreboard_refresh", [this]([[maybe_unused]] Registry &r, [[maybe_unused]] Entity e) {
        if (_onRefresh && _visible) {
            std::cout << "[ScoreboardMenu] Refresh clicked" << std::endl;
            _onRefresh();
        }
    });
}

void ScoreboardMenu::show() {
    _visible = true;
    std::cout << "[ScoreboardMenu] Showing scoreboard menu" << std::endl;

    // Show background
    _reg.getComponent<Sprite>(_backgroundPanel).visible = true;

    // Show title
    _reg.getComponent<Label>(_titleLabel).visible = true;

    // Show close button
    _reg.getComponent<Sprite>(_closeButton).visible = true;
    _reg.getComponent<Button>(_closeButton).enabled = true;

    // Show refresh button
    _reg.getComponent<Sprite>(_refreshButton).visible = true;
    _reg.getComponent<Button>(_refreshButton).enabled = true;

    // Show all score entries and headers
    for (auto &entity : _scoreEntities) {
        if (_reg.hasComponent<Label>(entity)) {
            _reg.getComponent<Label>(entity).visible = true;
        }
    }
}

void ScoreboardMenu::hide() {
    _visible = false;
    std::cout << "[ScoreboardMenu] Hiding scoreboard menu" << std::endl;

    // Hide background
    if (_reg.hasComponent<Sprite>(_backgroundPanel)) {
        _reg.getComponent<Sprite>(_backgroundPanel).visible = false;
    }

    // Hide title
    if (_reg.hasComponent<Label>(_titleLabel)) {
        _reg.getComponent<Label>(_titleLabel).visible = false;
    }

    // Hide close button
    if (_reg.hasComponent<Sprite>(_closeButton)) {
        _reg.getComponent<Sprite>(_closeButton).visible = false;
    }
    if (_reg.hasComponent<Button>(_closeButton)) {
        _reg.getComponent<Button>(_closeButton).enabled = false;
    }

    // Hide refresh button
    if (_reg.hasComponent<Sprite>(_refreshButton)) {
        _reg.getComponent<Sprite>(_refreshButton).visible = false;
    }
    if (_reg.hasComponent<Button>(_refreshButton)) {
        _reg.getComponent<Button>(_refreshButton).enabled = false;
    }

    // Hide all score entries and headers
    for (auto &entity : _scoreEntities) {
        if (_reg.hasComponent<Label>(entity)) {
            _reg.getComponent<Label>(entity).visible = false;
        }
    }
}

void ScoreboardMenu::clearScoreEntities() {
    // Keep the first few entities (headers, close/refresh labels)
    // Only clear dynamically added score entries (keep first 5)
    while (_scoreEntities.size() > 5) {
        Entity entity = _scoreEntities.back();
        _reg.destroyEntity(entity);
        _scoreEntities.pop_back();
    }
}

void ScoreboardMenu::createScoreEntry(size_t index, const ScoreEntry &entry) {
    float yPos = 210.f + (index * 35.f);

    // Rank number
    Entity rankLabel = _reg.createEntity();
    _reg.addComponent<Position>(rankLabel, 230.f, yPos);
    std::string rankText = std::to_string(index + 1) + ".";
    Color rankColor = (index == 0) ? Color(255, 215, 0) :   // Gold for 1st
                      (index == 1) ? Color(192, 192, 192) : // Silver for 2nd
                      (index == 2) ? Color(205, 127, 50) :  // Bronze for 3rd
                                     Color(255, 255, 255);   // White for others
    _reg.addComponent<Label>(rankLabel, rankText,
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("scoreboard_rank_" + std::to_string(index)), rankColor, 96, _visible);
    _scoreEntities.push_back(rankLabel);

    // Player name
    Entity nameLabel = _reg.createEntity();
    _reg.addComponent<Position>(nameLabel, 300.f, yPos);
    std::string displayName = entry.username;
    if (displayName.length() > 15) {
        displayName = displayName.substr(0, 12) + "...";
    }
    _reg.addComponent<Label>(nameLabel, displayName,
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("scoreboard_name_" + std::to_string(index)), rankColor, 96, _visible);
    _scoreEntities.push_back(nameLabel);

    // Score
    Entity scoreLabel = _reg.createEntity();
    _reg.addComponent<Position>(scoreLabel, 600.f, yPos);
    _reg.addComponent<Label>(scoreLabel, std::to_string(entry.highScore),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("scoreboard_score_" + std::to_string(index)), rankColor, 96, _visible);
    _scoreEntities.push_back(scoreLabel);
}

void ScoreboardMenu::updateScores(const std::vector<ScoreEntry> &scores) {
    _scores = scores;
    clearScoreEntities();

    std::cout << "[ScoreboardMenu] Updating with " << scores.size() << " scores" << std::endl;

    for (size_t i = 0; i < scores.size() && i < 10; ++i) {
        createScoreEntry(i, scores[i]);
    }

    if (scores.empty()) {
        // Show "No scores yet" message
        Entity noScoresLabel = _reg.createEntity();
        _reg.addComponent<Position>(noScoresLabel, 380.f, 320.f);
        _reg.addComponent<Label>(noScoresLabel, std::string("No scores yet"),
                                 std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                                 std::string("scoreboard_no_scores"), Color(150, 150, 150), 96, _visible);
        _scoreEntities.push_back(noScoresLabel);
    }
}
