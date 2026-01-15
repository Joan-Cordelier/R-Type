/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Room
*/

#ifndef ROOM_HPP_
#define ROOM_HPP_

#include "../../common/Data/ThreadedQueue.hpp"
#include "../Gameplay/GameHandler.hpp"
#include "../Session/SessionManager.hpp"
#include "../Monitoring/PrometheusExporter.hpp"
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

class Room {
public:
    Room(uint32_t id, SessionManager &session, const std::string &configPath, PrometheusExporter& monitor);
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

    // Push message to the GameHandler's queue
    void pushMessage(const DecodedMessage &msg);

    std::vector<uint32_t> getPlayers() const;

    // Returns true if the room has been empty for the specified duration
    bool hasBeenEmptyFor(std::chrono::seconds duration) const;

private:
    uint32_t _id;
    SessionManager &_session;
    std::atomic<bool> _running{false};
    PrometheusExporter& _monitor;

    std::shared_ptr<ThreadedQueue<DecodedMessage>> _inputQueue;
    std::unique_ptr<GameHandler> _game;

    std::thread _thread;

    mutable std::mutex _mutex;
    std::vector<uint32_t> _players;
    uint32_t _maxPlayers = 4;

    // Track when the room became empty (reset when players join)
    std::chrono::steady_clock::time_point _emptyTimestamp;
    bool _wasEmpty = true;  // Start as empty
};

#endif /* !ROOM_HPP_ */
