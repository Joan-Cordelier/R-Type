/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Room
*/

#ifndef ROOM_HPP_
#define ROOM_HPP_

#include "../../common/Data/RoomConfig.hpp"
#include "../../common/Data/ThreadedQueue.hpp"
#include "../Gameplay/GameHandler.hpp"
#include "../Monitoring/PrometheusExporter.hpp"
#include "../Session/SessionManager.hpp"
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

class Room {
public:
    Room(uint32_t id, SessionManager &session, const std::string &configPath,
         PrometheusExporter &monitor, const RoomConfig &config = RoomConfig());
    ~Room();

    void start();
    void stop();

    uint32_t getId() const {
        return _id;
    }
    size_t getPlayerCount() const;
    bool isFull() const;

    // Add player to room references
    void addPlayer(uint32_t playerId);
    void removePlayer(uint32_t playerId);
    void removePlayer(uint32_t playerId, uint32_t userId, const std::string &username);

    // Ban system - players who die or disconnect cannot rejoin
    void banPlayer(uint32_t playerId);
    bool isPlayerBanned(uint32_t playerId);

    // Called by GameHandler when a player dies
    void onPlayerDeath(uint32_t playerId);

    // Called by GameHandler when game is won (victory)
    void onGameVictory(uint32_t finalScore);

    // Push message to the GameHandler's queue
    void pushMessage(const DecodedMessage &msg);

    std::vector<uint32_t> getPlayers() const;

    // Returns true if the room has been empty for the specified duration
    bool hasBeenEmptyFor(std::chrono::seconds duration) const;

    // Get room configuration
    const RoomConfig &getConfig() const {
        return _config;
    }
    GameMode getGameMode() const {
        return _config.gameMode;
    }
    Difficulty getDifficulty() const {
        return _config.difficulty;
    }

private:
    uint32_t _id;
    SessionManager &_session;
    std::atomic<bool> _running{false};
    PrometheusExporter &_monitor;
    RoomConfig _config;

    std::shared_ptr<ThreadedQueue<DecodedMessage>> _inputQueue;
    std::unique_ptr<GameHandler> _game;

    std::thread _thread;

    mutable std::mutex _mutex;
    std::vector<uint32_t> _players;
    std::vector<uint32_t> _allParticipants; // All players who joined (for scoring)
    std::set<std::string>
        _bannedUsers; // User identifiers ("user:id" or "guest:name") banned from rejoining

    // Track when the room became empty (reset when players join)
    std::chrono::steady_clock::time_point _emptyTimestamp;
    bool _wasEmpty = true;      // Start as empty
    bool _gameComplete = false; // Set true when game is won - triggers immediate cleanup

public:
    // Check if game is complete (victory achieved) - room should be closed immediately
    bool isGameComplete() const {
        return _gameComplete;
    }
};

#endif /* !ROOM_HPP_ */
