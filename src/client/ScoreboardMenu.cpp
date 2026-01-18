/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ScoreboardMenu - Endless mode leaderboard with difficulty-based rankings
*/

#include "ScoreboardMenu.hpp"
#include <iostream>

ScoreboardMenu::ScoreboardMenu(Registry &reg) : _reg(reg) {
}

void ScoreboardMenu::init() {
    // Title label - "Endless Leaderboard"
    _titleLabel = _reg.createEntity();
    _reg.addComponent<Position>(_titleLabel, 280.f, 80.f);
    _reg.addComponent<Label>(_titleLabel, std::string("Endless Leaderboard"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font"), Color(255, 215, 0), 100, false);

    // Difficulty tabs - horizontal row
    const std::array<std::string, 4> diffNames = {"Easy", "Normal", "Hard", "Impossible"};
    const std::array<Color, 4> tabColors = {
        Color(100, 200, 100), // Easy - green
        Color(200, 200, 100), // Normal - yellow
        Color(200, 150, 100), // Hard - orange
        Color(200, 100, 100)  // Impossible - red
    };

    for (size_t i = 0; i < 4; ++i) {
        float xPos = 180.f + (i * 150.f);

        // Tab button
        _difficultyButtons[i] = _reg.createEntity();
        _reg.addComponent<Position>(_difficultyButtons[i], xPos, 140.f);
        _reg.addComponent<Sprite>(_difficultyButtons[i],
                                  std::string("textures/button/square_button.png"),
                                  std::string("square_button"), 130, 35, 100, 0.f, 0.f, false);
        _reg.addComponent<Button>(_difficultyButtons[i],
                                  std::string("leaderboard_diff_") + std::to_string(i), 100, false);

        // Tab label
        _difficultyLabels[i] = _reg.createEntity();
        _reg.addComponent<Position>(_difficultyLabels[i], xPos + 15.f, 148.f);
        _reg.addComponent<Label>(_difficultyLabels[i], diffNames[i],
                                 std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                                 std::string("default_font_small"), tabColors[i], 101, false);
    }

    // Column headers
    Entity rankHeader = _reg.createEntity();
    _reg.addComponent<Position>(rankHeader, 200.f, 195.f);
    _reg.addComponent<Label>(rankHeader, std::string("#"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(180, 180, 180), 100, false);
    _scoreEntities.push_back(rankHeader);

    Entity nameHeader = _reg.createEntity();
    _reg.addComponent<Position>(nameHeader, 260.f, 195.f);
    _reg.addComponent<Label>(nameHeader, std::string("Player"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(180, 180, 180), 100, false);
    _scoreEntities.push_back(nameHeader);

    Entity scoreHeader = _reg.createEntity();
    _reg.addComponent<Position>(scoreHeader, 580.f, 195.f);
    _reg.addComponent<Label>(scoreHeader, std::string("Score"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(180, 180, 180), 100, false);
    _scoreEntities.push_back(scoreHeader);

    // Close button
    _closeButton = _reg.createEntity();
    _reg.addComponent<Position>(_closeButton, 380.f, 520.f);
    _reg.addComponent<Sprite>(_closeButton, std::string("textures/button/square_button.png"),
                              std::string("square_button"), 150, 45, 100, 0.f, 0.f, false);
    _reg.addComponent<Button>(_closeButton, std::string("leaderboard_close"), 100, false);

    _closeButtonLabel = _reg.createEntity();
    _reg.addComponent<Position>(_closeButtonLabel, 425.f, 532.f);
    _reg.addComponent<Label>(_closeButtonLabel, std::string("Close"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 255, 255), 101, false);

    std::cout << "[ScoreboardMenu] Initialized (Endless Leaderboard)" << std::endl;
}

void ScoreboardMenu::setup(ButtonSystem &buttonsys) {
    buttonsys.registerHandler("leaderboard_close",
                              [this]([[maybe_unused]] Registry &r, [[maybe_unused]] Entity e) {
                                  if (_onClose && _visible) {
                                      std::cout << "[ScoreboardMenu] Close clicked" << std::endl;
                                      _onClose();
                                  }
                              });

    // Register difficulty tab handlers
    for (int i = 0; i < 4; ++i) {
        std::string handlerName = "leaderboard_diff_" + std::to_string(i);
        buttonsys.registerHandler(
            handlerName, [this, i]([[maybe_unused]] Registry &r, [[maybe_unused]] Entity e) {
                if (_visible && _currentDifficulty != static_cast<uint8_t>(i)) {
                    _currentDifficulty = static_cast<uint8_t>(i);
                    std::cout << "[ScoreboardMenu] Switched to "
                              << getDifficultyName(_currentDifficulty) << std::endl;
                    updateDifficultyHighlight();
                    if (_onDifficultyChange) {
                        _onDifficultyChange(_currentDifficulty);
                    }
                }
            });
    }
}

std::string ScoreboardMenu::getDifficultyName(uint8_t diff) const {
    switch (diff) {
    case 0:
        return "Easy";
    case 1:
        return "Normal";
    case 2:
        return "Hard";
    case 3:
        return "Impossible";
    default:
        return "Unknown";
    }
}

void ScoreboardMenu::updateDifficultyHighlight() {
    const std::array<Color, 4> normalColors = {Color(100, 200, 100), Color(200, 200, 100),
                                               Color(200, 150, 100), Color(200, 100, 100)};

    for (size_t i = 0; i < 4; ++i) {
        if (_reg.hasComponent<Label>(_difficultyLabels[i])) {
            auto &label = _reg.getComponent<Label>(_difficultyLabels[i]);
            if (i == _currentDifficulty) {
                // Active tab - bright white
                label.color = Color(255, 255, 255);
            } else {
                // Inactive tab - dimmed color
                label.color = normalColors[i];
            }
        }
    }
}

void ScoreboardMenu::show() {
    _visible = true;
    std::cout << "[ScoreboardMenu] Showing endless leaderboard" << std::endl;

    // Show title
    _reg.getComponent<Label>(_titleLabel).visible = true;

    // Show difficulty tabs
    for (size_t i = 0; i < 4; ++i) {
        _reg.getComponent<Sprite>(_difficultyButtons[i]).visible = true;
        _reg.getComponent<Button>(_difficultyButtons[i]).enabled = true;
        _reg.getComponent<Label>(_difficultyLabels[i]).visible = true;
    }

    // Show close button
    _reg.getComponent<Sprite>(_closeButton).visible = true;
    _reg.getComponent<Button>(_closeButton).enabled = true;
    _reg.getComponent<Label>(_closeButtonLabel).visible = true;

    // Show all score entries
    for (auto &entity : _scoreEntities) {
        if (_reg.hasComponent<Label>(entity)) {
            _reg.getComponent<Label>(entity).visible = true;
        }
    }

    updateDifficultyHighlight();
}

void ScoreboardMenu::hide() {
    _visible = false;
    std::cout << "[ScoreboardMenu] Hiding leaderboard" << std::endl;

    // Hide title
    if (_reg.hasComponent<Label>(_titleLabel)) {
        _reg.getComponent<Label>(_titleLabel).visible = false;
    }

    // Hide difficulty tabs
    for (size_t i = 0; i < 4; ++i) {
        if (_reg.hasComponent<Sprite>(_difficultyButtons[i])) {
            _reg.getComponent<Sprite>(_difficultyButtons[i]).visible = false;
        }
        if (_reg.hasComponent<Button>(_difficultyButtons[i])) {
            _reg.getComponent<Button>(_difficultyButtons[i]).enabled = false;
        }
        if (_reg.hasComponent<Label>(_difficultyLabels[i])) {
            _reg.getComponent<Label>(_difficultyLabels[i]).visible = false;
        }
    }

    // Hide close button
    if (_reg.hasComponent<Sprite>(_closeButton)) {
        _reg.getComponent<Sprite>(_closeButton).visible = false;
    }
    if (_reg.hasComponent<Button>(_closeButton)) {
        _reg.getComponent<Button>(_closeButton).enabled = false;
    }
    if (_reg.hasComponent<Label>(_closeButtonLabel)) {
        _reg.getComponent<Label>(_closeButtonLabel).visible = false;
    }

    // Hide all score entries
    for (auto &entity : _scoreEntities) {
        if (_reg.hasComponent<Label>(entity)) {
            _reg.getComponent<Label>(entity).visible = false;
        }
    }
}

void ScoreboardMenu::clearScoreEntities() {
    // Keep the first 3 entities (headers)
    while (_scoreEntities.size() > 3) {
        Entity entity = _scoreEntities.back();
        _reg.destroyEntity(entity);
        _scoreEntities.pop_back();
    }
}

void ScoreboardMenu::createScoreEntry(size_t index, const ScoreEntry &entry) {
    float yPos = 230.f + (index * 30.f);

    // Rank color (gold/silver/bronze for top 3)
    Color rankColor = (index == 0) ? Color(255, 215, 0) : // Gold
                          (index == 1) ? Color(192, 192, 192)
                                       : // Silver
                          (index == 2) ? Color(205, 127, 50)
                                       :        // Bronze
                          Color(255, 255, 255); // White

    // Rank number
    Entity rankLabel = _reg.createEntity();
    _reg.addComponent<Position>(rankLabel, 200.f, yPos);
    std::string rankText = std::to_string(index + 1) + ".";
    _reg.addComponent<Label>(rankLabel, rankText,
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), rankColor, 100, _visible);
    _scoreEntities.push_back(rankLabel);

    // Player name (truncate if too long)
    Entity nameLabel = _reg.createEntity();
    _reg.addComponent<Position>(nameLabel, 260.f, yPos);
    std::string displayName = entry.username;
    if (displayName.length() > 18) {
        displayName = displayName.substr(0, 15) + "...";
    }
    _reg.addComponent<Label>(nameLabel, displayName,
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), rankColor, 100, _visible);
    _scoreEntities.push_back(nameLabel);

    // Score
    Entity scoreLabel = _reg.createEntity();
    _reg.addComponent<Position>(scoreLabel, 580.f, yPos);
    _reg.addComponent<Label>(scoreLabel, std::to_string(entry.highScore),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), rankColor, 100, _visible);
    _scoreEntities.push_back(scoreLabel);
}

void ScoreboardMenu::updateScores(const std::vector<ScoreEntry> &scores, uint8_t difficulty) {
    _scores = scores;
    _currentDifficulty = difficulty;
    clearScoreEntities();

    std::cout << "[ScoreboardMenu] Updating with " << scores.size() << " scores for "
              << getDifficultyName(difficulty) << std::endl;

    for (size_t i = 0; i < scores.size() && i < 10; ++i) {
        createScoreEntry(i, scores[i]);
    }

    if (scores.empty()) {
        // Show "No scores yet" message
        Entity noScoresLabel = _reg.createEntity();
        _reg.addComponent<Position>(noScoresLabel, 340.f, 350.f);
        _reg.addComponent<Label>(noScoresLabel, std::string("No scores yet"),
                                 std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                                 std::string("default_font_small"), Color(150, 150, 150), 100,
                                 _visible);
        _scoreEntities.push_back(noScoresLabel);
    }

    updateDifficultyHighlight();
}
