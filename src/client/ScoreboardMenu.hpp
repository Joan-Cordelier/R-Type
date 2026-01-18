/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ScoreboardMenu - Client scoreboard UI for displaying top scores
*/

#ifndef SCOREBOARDMENU_HPP_
#define SCOREBOARDMENU_HPP_

#include "../common/ecs/registry.hpp"
#include "../common/ecs/ecs_components.hpp"
#include "../common/ecs/ecs_system.hpp"
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
    using RefreshCallback = std::function<void()>;

    ScoreboardMenu(Registry &reg);
    ~ScoreboardMenu() = default;

    void init();
    void setup(ButtonSystem &buttonsys);
    void show();
    void hide();
    bool isVisible() const { return _visible; }

    void updateScores(const std::vector<ScoreEntry> &scores);
    void setCloseCallback(CloseCallback callback) { _onClose = callback; }
    void setRefreshCallback(RefreshCallback callback) { _onRefresh = callback; }

private:
    Registry &_reg;
    bool _visible = false;

    // UI Entities
    Entity _backgroundPanel;
    Entity _titleLabel;
    Entity _closeButton;
    Entity _refreshButton;
    std::vector<Entity> _scoreEntities;

    // Score data
    std::vector<ScoreEntry> _scores;

    // Callbacks
    CloseCallback _onClose;
    RefreshCallback _onRefresh;

    void clearScoreEntities();
    void createScoreEntry(size_t index, const ScoreEntry &entry);
};

#endif /* !SCOREBOARDMENU_HPP_ */
