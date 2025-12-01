/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SessionManager
*/

#include "SessionManager.hpp"
#include "../Logs/Logger.hpp"

SessionManager::SessionManager() 
    : _tcpServer(_queue), _udpServer(_queue)
{
}

SessionManager::~SessionManager()
{
    stop();
}

void SessionManager::start()
{
    if (_running)
        return;
    
    _running = true;
    LOG_INFO("Starting SessionManager...");
    
    _tcpThread = std::thread(&TCPServer::run, &_tcpServer);
    _udpThread = std::thread(&UDPServer::run, &_udpServer);
    
    LOG_INFO("SessionManager started - TCP and UDP servers running");
}

void SessionManager::stop()
{
    if (!_running)
        return;
    
    LOG_INFO("Stopping SessionManager...");
    _running = false;
    
    _tcpServer.stop();
    _udpServer.stop();
    
    if (_tcpThread.joinable())
        _tcpThread.join();
    if (_udpThread.joinable())
        _udpThread.join();
    
    LOG_INFO("SessionManager stopped");
}

// Player Management

uint32_t SessionManager::addPlayer(int tcpFd)
{
    std::lock_guard<std::mutex> lock(_playersMutex);
    
    uint32_t playerId = _nextPlayerId++;
    _players.emplace(playerId, Player(playerId, tcpFd));
    _tcpFdToPlayerId[tcpFd] = playerId;
    
    LOG_INFO("Player " + std::to_string(playerId) + " added (TCP fd: " + std::to_string(tcpFd) + ")");
    return playerId;
}

void SessionManager::linkPlayerUdp(uint32_t playerId, const sockaddr_in& addr)
{
    std::lock_guard<std::mutex> lock(_playersMutex);
    
    auto it = _players.find(playerId);
    if (it != _players.end()) {
        it->second.udpAddr = addr;
        it->second.udpLinked = true;
        LOG_INFO("Player " + std::to_string(playerId) + " linked to UDP");
    }
}

void SessionManager::removePlayer(uint32_t playerId)
{
    std::lock_guard<std::mutex> lock(_playersMutex);
    
    auto it = _players.find(playerId);
    if (it != _players.end()) {
        _tcpFdToPlayerId.erase(it->second.tcpFd);
        _players.erase(it);
        LOG_INFO("Player " + std::to_string(playerId) + " removed");
    }
}

void SessionManager::removePlayerByTcpFd(int tcpFd)
{
    std::lock_guard<std::mutex> lock(_playersMutex);
    
    auto it = _tcpFdToPlayerId.find(tcpFd);
    if (it != _tcpFdToPlayerId.end()) {
        uint32_t playerId = it->second;
        _players.erase(playerId);
        _tcpFdToPlayerId.erase(it);
        LOG_INFO("Player " + std::to_string(playerId) + " removed (by TCP fd)");
    }
}

Player* SessionManager::getPlayer(uint32_t playerId)
{
    std::lock_guard<std::mutex> lock(_playersMutex);
    
    auto it = _players.find(playerId);
    return (it != _players.end()) ? &it->second : nullptr;
}

Player* SessionManager::getPlayerByTcpFd(int tcpFd)
{
    std::lock_guard<std::mutex> lock(_playersMutex);
    
    auto it = _tcpFdToPlayerId.find(tcpFd);
    if (it != _tcpFdToPlayerId.end()) {
        auto playerIt = _players.find(it->second);
        return (playerIt != _players.end()) ? &playerIt->second : nullptr;
    }
    return nullptr;
}

Player* SessionManager::getPlayerByUdpAddr(const sockaddr_in& addr)
{
    std::lock_guard<std::mutex> lock(_playersMutex);
    
    for (auto& [id, player] : _players) {
        if (player.udpLinked && compareUdpAddr(player.udpAddr, addr))
            return &player;
    }
    return nullptr;
}

std::vector<Player*> SessionManager::getPlayersInRoom(uint32_t roomId)
{
    std::lock_guard<std::mutex> lock(_playersMutex);
    
    std::vector<Player*> result;
    for (auto& [id, player] : _players) {
        if (player.roomId == roomId)
            result.push_back(&player);
    }
    return result;
}

std::vector<Player*> SessionManager::getAllPlayers()
{
    std::lock_guard<std::mutex> lock(_playersMutex);
    
    std::vector<Player*> result;
    for (auto& [id, player] : _players)
        result.push_back(&player);
    return result;
}

size_t SessionManager::getPlayerCount() const
{
    std::lock_guard<std::mutex> lock(_playersMutex);
    return _players.size();
}

// Network - Send to specific player

void SessionManager::sendTcp(uint32_t playerId, const MessageData& data)
{
    std::lock_guard<std::mutex> lock(_playersMutex);
    
    auto it = _players.find(playerId);
    if (it != _players.end() && it->second.tcpFd >= 0) {
        sockaddr_in dummy{};
        _tcpServer.send(data, dummy);  // TCP sends to fd internally
    }
}

void SessionManager::sendUdp(uint32_t playerId, const MessageData& data)
{
    std::lock_guard<std::mutex> lock(_playersMutex);
    
    auto it = _players.find(playerId);
    if (it != _players.end() && it->second.udpLinked) {
        _udpServer.send(data, it->second.udpAddr);
    }
}

// Network - Broadcast

void SessionManager::broadcastTcp(const MessageData& data, uint32_t roomId)
{
    if (roomId == 0) {
        _tcpServer.send(data);  // Broadcast to all
    } else {
        std::lock_guard<std::mutex> lock(_playersMutex);
        for (auto& [id, player] : _players) {
            if (player.roomId == roomId && player.tcpFd >= 0) {
                // Send to specific room - would need per-fd send
                // For now, this is simplified
            }
        }
    }
}

void SessionManager::broadcastUdp(const MessageData& data, uint32_t roomId)
{
    if (roomId == 0) {
        _udpServer.send(data);  // Broadcast to all
    } else {
        std::lock_guard<std::mutex> lock(_playersMutex);
        for (auto& [id, player] : _players) {
            if (player.roomId == roomId && player.udpLinked) {
                _udpServer.send(data, player.udpAddr);
            }
        }
    }
}

// Queue access

std::optional<DecodedMessage> SessionManager::popMessage(Priority priority)
{
    return _queue.pop(priority);
}

bool SessionManager::hasMessages() const
{
    return !_queue.isEmpty();
}

// Helper

bool SessionManager::compareUdpAddr(const sockaddr_in& a, const sockaddr_in& b) const
{
    return a.sin_addr.s_addr == b.sin_addr.s_addr && a.sin_port == b.sin_port;
}
