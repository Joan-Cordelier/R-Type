/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Room - A single game room with its own network and game state
*/

#ifndef ROOM_HPP_
#define ROOM_HPP_

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>

#include "../Session/SessionManager.hpp"

class RoomGameHandler;

/**
 * @brief A single game room with dedicated network and game state.
 *
 * Each Room owns:
 * - Its own SessionManager (TCP + UDP on dynamic ports)
 * - Its own RoomGameHandler (ECS world, game logic)
 *
 * The Room runs its network and game loop on a dedicated thread.
 */
class Room {
public:
    Room(uint8_t id, const std::string &name);
    ~Room();

    // Lifecycle
    void start();
    void stop();
    bool isRunning() const { return _running; }
    bool isStarted() const { return _gameStarted; }

    // Get network ports (call after start())
    uint16_t getTcpPort() const;
    uint16_t getUdpPort() const;

    // Player management (for room's internal tracking)
    void addPlayer(uint32_t playerId, int tcpFd);
    void removePlayer(uint32_t playerId);
    bool hasPlayer(uint32_t playerId) const;
    std::set<uint32_t> getPlayerIds() const;
    size_t getPlayerCount() const;

    // Link player UDP (called when player sends LINK on room's UDP)
    void linkPlayerUdp(uint32_t playerId, const sockaddr_in &addr);

    // Getters
    uint8_t getId() const { return _id; }
    std::string getName() const { return _name; }

private:
    void runLoop(); // Main room thread loop
    void processMessages();
    void handleMessage(const DecodedMessage &msg);

    uint8_t _id;
    std::string _name;

    // Room's own network (TCP + UDP on dynamic ports)
    std::unique_ptr<SessionManager> _session;

    // Game handler
    std::unique_ptr<RoomGameHandler> _game;

    // Room thread
    std::thread _roomThread;
    std::atomic<bool> _running{false};
    std::atomic<bool> _gameStarted{false};

    // Player tracking (mirrors _session but for quick lookup)
    std::set<uint32_t> _playerIds;
    mutable std::mutex _playersMutex;
};

#endif /* !ROOM_HPP_ */
