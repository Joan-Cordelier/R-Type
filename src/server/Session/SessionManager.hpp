/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SessionManager - Reusable network session manager
*/

#ifndef SESSIONMANAGER_HPP_
#define SESSIONMANAGER_HPP_

#include "../../common/Data/ThreadedQueue.hpp"
#include "../Network/TCPServer.hpp"
#include "../Network/UDPServer.hpp"
#include "Player.hpp"
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

/**
 * @brief Configuration for SessionManager initialization.
 */
struct SessionConfig {
    int tcpPort = 0; // 0 = dynamic port assignment
    int udpPort = 0; // 0 = dynamic port, -1 = no UDP
    bool enableUdp = true;

    // Static configs for common use cases
    static SessionConfig lobby() {
        return {4789, -1, false}; // Lobby: TCP only, fixed port
    }

    static SessionConfig room() {
        return {0, 0, true}; // Room: Dynamic TCP + UDP
    }
};

/**
 * @brief Reusable network session manager.
 *
 * Can be configured for different use cases:
 * - Lobby: TCP only on fixed port for auth/room management
 * - Room: TCP + UDP on dynamic ports for game traffic
 */
class SessionManager {
public:
    using PlayerCallback = std::function<void(const Player &)>;

    // Default constructor (for backward compatibility - uses fixed ports
    // 4789/4790)
    SessionManager();

    // Configurable constructor
    explicit SessionManager(const SessionConfig &config);

    ~SessionManager();

    void start();
    void stop();
    bool isRunning() const { return _running; }

    // Get bound ports (call after start())
    uint16_t getTcpPort() const;
    uint16_t getUdpPort() const; // Returns 0 if UDP disabled

    void setOnPlayerDisconnect(PlayerCallback callback) {
        _onPlayerDisconnect = callback;
    }
    void setOnPlayerConnect(PlayerCallback callback) {
        _onPlayerConnect = callback;
    }

    uint32_t addPlayer(int tcpFd);
    void linkPlayerUdp(uint32_t playerId, const sockaddr_in &addr);
    void setPlayerRoom(uint32_t playerId, uint8_t roomId);
    void updatePlayerId(uint32_t oldId, uint32_t newId);
    void removePlayer(uint32_t playerId);
    void removePlayerByTcpFd(int tcpFd);

    Player *getPlayer(uint32_t playerId);
    Player *getPlayerByTcpFd(int tcpFd);
    Player *getPlayerByUdpAddr(const sockaddr_in &addr);
    std::vector<Player *> getPlayersInRoom(uint8_t roomId);
    std::vector<Player *> getAllPlayers();
    size_t getPlayerCount() const;

    void sendTcp(uint32_t playerId, const MessageData &data,
                 Priority priority = Priority::MEDIUM);
    void sendUdp(uint32_t playerId, const MessageData &data);

    void sendTcp(uint32_t playerId, const PreparedMessage &msg);
    void sendUdp(uint32_t playerId, const PreparedMessage &msg);

    void broadcastTcp(const MessageData &data, uint8_t roomId = 0);
    void broadcastUdp(const MessageData &data, uint8_t roomId = 0);

    void broadcastTcp(const PreparedMessage &msg, uint8_t roomId = 0);
    void broadcastUdp(const PreparedMessage &msg, uint8_t roomId = 0);

    std::optional<DecodedMessage> popMessage(Priority priority);
    bool hasMessages() const;

    TCPServer &getTcpServer() { return _tcpServer; }
    UDPServer *getUdpServer() { return _udpServer.get(); } // May be null
    ThreadedQueue<DecodedMessage> &getQueue() { return _queue; }

private:
    SessionConfig _config;

    ThreadedQueue<DecodedMessage> _queue;
    TCPServer _tcpServer;
    std::unique_ptr<UDPServer> _udpServer; // Optional (null if UDP disabled)

    std::thread _tcpThread;
    std::thread _udpThread;
    std::atomic<bool> _running{false};

    std::unordered_map<uint32_t, Player> _players;
    std::unordered_map<int, uint32_t> _tcpFdToPlayerId;
    mutable std::mutex _playersMutex;
    uint32_t _nextPlayerId = 1;

    PlayerCallback _onPlayerDisconnect;
    PlayerCallback _onPlayerConnect;

    void setupCallbacks();
    bool compareUdpAddr(const sockaddr_in &a, const sockaddr_in &b) const;
};

#endif /* !SESSIONMANAGER_HPP_ */
