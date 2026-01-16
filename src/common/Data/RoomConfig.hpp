/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RoomConfig - Room configuration structure
*/

#ifndef ROOMCONFIG_HPP_
#define ROOMCONFIG_HPP_

#include <cstdint>
#include <string>

enum class GameMode : uint8_t {
    CLASSIC = 0,      // Normal wave-based gameplay
    BOSS_RUSH = 1,    // Only boss fights
    ENDLESS = 2,      // Infinite waves with increasing difficulty
    SPEEDY = 3        // Survival mode with faster enemies
};

enum class Difficulty : uint8_t {
    EASY = 0,
    NORMAL = 1,
    HARD = 2,
    IMPOSSIBLE = 3
};

struct RoomConfig {
    uint8_t maxPlayers = 4;
    GameMode gameMode = GameMode::CLASSIC;
    Difficulty difficulty = Difficulty::NORMAL;

    // Helper to get string representations
    static std::string gameModeToString(GameMode mode) {
        switch (mode) {
            case GameMode::CLASSIC:   return "Classic";
            case GameMode::BOSS_RUSH: return "Boss Rush";
            case GameMode::ENDLESS:   return "Endless";
            case GameMode::SPEEDY:    return "Speedy";
            default:                  return "Unknown";
        }
    }

    static std::string difficultyToString(Difficulty diff) {
        switch (diff) {
            case Difficulty::EASY:      return "Easy";
            case Difficulty::NORMAL:    return "Normal";
            case Difficulty::HARD:      return "Hard";
            case Difficulty::IMPOSSIBLE: return "Impossible";
            default:                    return "Unknown";
        }
    }

    std::string getGameModeStr() const { return gameModeToString(gameMode); }
    std::string getDifficultyStr() const { return difficultyToString(difficulty); }
};

#endif /* !ROOMCONFIG_HPP_ */
