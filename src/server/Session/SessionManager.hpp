/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SessionManager
*/

#ifndef SESSIONMANAGER_HPP_
#define SESSIONMANAGER_HPP_

#include "Player.hpp"
#include "../Network/TCPServer.hpp"
#include "../Network/UDPServer.hpp"
#include "../../common/Data/ThreadedQueue.hpp"
#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>

class SessionManager {
    public:
        SessionManager();
        ~SessionManager();
        
        // Lifecycle
        void start();
        void stop();
        bool isRunning() const { return _running; }
        
        // Player management
        uint32_t addPlayer(int tcpFd);
        void linkPlayerUdp(uint32_t playerId, const sockaddr_in& addr);
        void removePlayer(uint32_t playerId);
        void removePlayerByTcpFd(int tcpFd);
        
        Player* getPlayer(uint32_t playerId);
        Player* getPlayerByTcpFd(int tcpFd);
        Player* getPlayerByUdpAddr(const sockaddr_in& addr);
        std::vector<Player*> getPlayersInRoom(uint32_t roomId);
        std::vector<Player*> getAllPlayers();
        size_t getPlayerCount() const;
        
        // Network - send to specific player
        void sendTcp(uint32_t playerId, const MessageData& data);
        void sendUdp(uint32_t playerId, const MessageData& data);
        
        // Network - broadcast
        void broadcastTcp(const MessageData& data, uint32_t roomId = 0);
        void broadcastUdp(const MessageData& data, uint32_t roomId = 0);
        
        // Queue access
        std::optional<DecodedMessage> popMessage(Priority priority);
        bool hasMessages() const;
        
        // Direct server access (if needed)
        TCPServer& getTcpServer() { return _tcpServer; }
        UDPServer& getUdpServer() { return _udpServer; }
        ThreadedQueue<DecodedMessage>& getQueue() { return _queue; }

    private:
        // Core components
        ThreadedQueue<DecodedMessage> _queue;
        TCPServer _tcpServer;
        UDPServer _udpServer;
        
        // Threads
        std::thread _tcpThread;
        std::thread _udpThread;
        std::atomic<bool> _running{false};
        
        // Players
        std::unordered_map<uint32_t, Player> _players;
        std::unordered_map<int, uint32_t> _tcpFdToPlayerId;
        mutable std::mutex _playersMutex;
        uint32_t _nextPlayerId = 1;
        
        // Helper
        bool compareUdpAddr(const sockaddr_in& a, const sockaddr_in& b) const;
};

#endif /* !SESSIONMANAGER_HPP_ */
