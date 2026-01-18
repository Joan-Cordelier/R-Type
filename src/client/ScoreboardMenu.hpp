/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ScoreboardMenu - Endless mode leaderboard with difficulty-based rankings
*/

#ifndef SCOREBOARDMENU_HPP_
#define SCOREBOARDMENU_HPP_

#include "../common/ecs/ecs_components.hpp"
#include "../common/ecs/ecs_system.hpp"
#include "../common/ecs/registry.hpp"
#include <array>
#include <functional>
#include <string>
#include <vector>

struct ScoreEntry {
    std::string username;
    uint32_t highScore;
};

class ScoreboardMenu {
public:
    using CloseCallback = std::function<void()>;
    using DifficultyCallback = std::function<void(uint8_t)>; // Called when difficulty tab clicked

    ScoreboardMenu(Registry &reg);
    ~ScoreboardMenu() = default;

    void init();
    void setup(ButtonSystem &buttonsys);
    void show();
    void hide();
    bool isVisible() const {
        return _visible;
    }

    void updateScores(const std::vector<ScoreEntry> &scores, uint8_t difficulty);
    void setCloseCallback(CloseCallback callback) {
        _onClose = callback;
    }
    void setDifficultyCallback(DifficultyCallback callback) {
        _onDifficultyChange = callback;
    }

    uint8_t getCurrentDifficulty() const {
        return _currentDifficulty;
    }

private:
    Registry &_reg;
    bool _visible = false;
    uint8_t _currentDifficulty = 1; // Default to Normal (0=Easy, 1=Normal, 2=Hard, 3=Impossible)

    // UI Entities
    Entity _titleLabel;
    Entity _closeButton;
    Entity _closeButtonLabel;
    std::array<Entity, 4> _difficultyButtons;
    std::array<Entity, 4> _difficultyLabels;
    Entity _difficultyIndicator; // Highlight for current tab
    std::vector<Entity> _scoreEntities;

    // Score data
    std::vector<ScoreEntry> _scores;

    // Callbacks
    CloseCallback _onClose;
    DifficultyCallback _onDifficultyChange;

    void clearScoreEntities();
    void createScoreEntry(size_t index, const ScoreEntry &entry);
    void updateDifficultyHighlight();
    std::string getDifficultyName(uint8_t diff) const;
};

#endif /* !SCOREBOARDMENU_HPP_ */
