/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SessionManager - Reusable network session manager
*/

#include "SessionManager.hpp"
#include "../Logs/Logger.hpp"

// Default constructor - backward compatible (fixed ports 4789/4790)
SessionManager::SessionManager()
    : _config{4789, 4790, true}, _tcpServer(_queue),
      _udpServer(std::make_unique<UDPServer>(_queue)) {
    setupCallbacks();
}

// Configurable constructor
SessionManager::SessionManager(const SessionConfig &config)
    : _config(config), _tcpServer(_queue),
      _udpServer(config.enableUdp ? std::make_unique<UDPServer>(_queue)
                                  : nullptr) {
    setupCallbacks();
}

void SessionManager::setupCallbacks() {
    _tcpServer.setOnConnect([this](int fd) {
        uint32_t playerId = addPlayer(fd);
        // Callback is called inside addPlayer
        (void)playerId;
    });

    _tcpServer.setOnDisconnect([this](int fd) { removePlayerByTcpFd(fd); });
}

SessionManager::~SessionManager() { stop(); }

void SessionManager::start() {
    if (_running)
        return;

    // Initialize TCP
    if (_tcpServer.init(AServer::TCP, _config.tcpPort) != 0) {
        LOG_ERROR("SessionManager: Failed to init TCP on port " +
                  std::to_string(_config.tcpPort));
        return;
    }

    // Initialize UDP if enabled
    if (_config.enableUdp && _udpServer) {
        if (_udpServer->init(AServer::UDP, _config.udpPort) != 0) {
            LOG_ERROR("SessionManager: Failed to init UDP on port " +
                      std::to_string(_config.udpPort));
            return;
        }
    }

    _running = true;
    LOG_INFO("Starting SessionManager...");

    _tcpThread = std::thread(&TCPServer::run, &_tcpServer);

    if (_config.enableUdp && _udpServer) {
        _udpThread = std::thread(&UDPServer::run, _udpServer.get());
        LOG_INFO("SessionManager started (TCP:" + std::to_string(getTcpPort()) +
                 ", UDP:" + std::to_string(getUdpPort()) + ")");
    } else {
        LOG_INFO("SessionManager started (TCP:" + std::to_string(getTcpPort()) +
                 ", UDP: disabled)");
    }
}

void SessionManager::stop() {
    if (!_running)
        return;

    LOG_INFO("Stopping SessionManager...");
    _running = false;

    _tcpServer.stop();
    if (_udpServer) {
        _udpServer->stop();
    }

    if (_tcpThread.joinable())
        _tcpThread.join();
    if (_udpThread.joinable())
        _udpThread.join();

    LOG_INFO("SessionManager stopped");
}

uint16_t SessionManager::getTcpPort() const { return _tcpServer.getPort(); }

uint16_t SessionManager::getUdpPort() const {
    return _udpServer ? _udpServer->getPort() : 0;
}

uint32_t SessionManager::addPlayer(int tcpFd) {
    std::lock_guard<std::mutex> lock(_playersMutex);

    uint32_t playerId = _nextPlayerId++;
    _players.emplace(playerId, Player(playerId, tcpFd));
    _tcpFdToPlayerId[tcpFd] = playerId;

    LOG_INFO("Player " + std::to_string(playerId) +
             " added (TCP fd: " + std::to_string(tcpFd) + ")");

    if (_onPlayerConnect) {
        _onPlayerConnect(_players.at(playerId));
    }

    return playerId;
}

void SessionManager::linkPlayerUdp(uint32_t playerId, const sockaddr_in &addr) {
    if (!_config.enableUdp) {
        LOG_WARN("linkPlayerUdp called but UDP is disabled");
        return;
    }

    std::lock_guard<std::mutex> lock(_playersMutex);

    auto it = _players.find(playerId);
    if (it != _players.end()) {
        it->second.udpAddr = addr;
        it->second.udpLinked = true;
        LOG_INFO("Player " + std::to_string(playerId) + " linked to UDP");
    }
}

void SessionManager::setPlayerRoom(uint32_t playerId, uint8_t roomId) {
    std::lock_guard<std::mutex> lock(_playersMutex);

    auto it = _players.find(playerId);
    if (it != _players.end()) {
        it->second.roomId = roomId;
        LOG_DEBUG("Player " + std::to_string(playerId) + " assigned to room " +
                  std::to_string(roomId));
    }
}

void SessionManager::updatePlayerId(uint32_t oldId, uint32_t newId) {
    if (oldId == newId)
        return;

    std::lock_guard<std::mutex> lock(_playersMutex);

    // Ensure old ID exists
    auto it = _players.find(oldId);
    if (it == _players.end()) {
        LOG_WARN("Cannot update player ID: old ID " + std::to_string(oldId) +
                 " not found");
        return;
    }

    // Ensure new ID does not exist
    if (_players.find(newId) != _players.end()) {
        LOG_WARN("Cannot update player ID: new ID " + std::to_string(newId) +
                 " already exists");
        return;
    }

    // Move player data to new ID
    Player player = std::move(it->second);
    player.id = newId;

    _players.erase(it);
    _players.emplace(newId, std::move(player));

    // Update TCP map
    for (auto &pair : _tcpFdToPlayerId) {
        if (pair.second == oldId) {
            pair.second = newId;
            break;
        }
    }

    LOG_INFO("Updated player ID from " + std::to_string(oldId) + " to " +
             std::to_string(newId));
}

void SessionManager::removePlayer(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(_playersMutex);

    auto it = _players.find(playerId);
    if (it != _players.end()) {
        _tcpFdToPlayerId.erase(it->second.tcpFd);
        _players.erase(it);
        LOG_INFO("Player " + std::to_string(playerId) + " removed");
    }
}

void SessionManager::removePlayerByTcpFd(int tcpFd) {
    std::lock_guard<std::mutex> lock(_playersMutex);

    auto it = _tcpFdToPlayerId.find(tcpFd);
    if (it != _tcpFdToPlayerId.end()) {
        uint32_t playerId = it->second;
        auto playerIt = _players.find(playerId);
        if (playerIt != _players.end()) {
            if (_onPlayerDisconnect) {
                _onPlayerDisconnect(playerIt->second);
            }
            _players.erase(playerIt);
        }
        _tcpFdToPlayerId.erase(it);
        LOG_INFO("Player " + std::to_string(playerId) + " removed (by TCP fd)");
    }
}

Player *SessionManager::getPlayer(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(_playersMutex);

    auto it = _players.find(playerId);
    return (it != _players.end()) ? &it->second : nullptr;
}

Player *SessionManager::getPlayerByTcpFd(int tcpFd) {
    std::lock_guard<std::mutex> lock(_playersMutex);

    auto it = _tcpFdToPlayerId.find(tcpFd);
    if (it != _tcpFdToPlayerId.end()) {
        auto playerIt = _players.find(it->second);
        return (playerIt != _players.end()) ? &playerIt->second : nullptr;
    }
    return nullptr;
}

Player *SessionManager::getPlayerByUdpAddr(const sockaddr_in &addr) {
    if (!_config.enableUdp) {
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(_playersMutex);

    for (auto &[id, player] : _players) {
        if (player.udpLinked && compareUdpAddr(player.udpAddr, addr))
            return &player;
    }
    return nullptr;
}

std::vector<Player *> SessionManager::getPlayersInRoom(uint8_t roomId) {
    std::lock_guard<std::mutex> lock(_playersMutex);

    std::vector<Player *> result;
    for (auto &[id, player] : _players) {
        if (player.roomId == roomId)
            result.push_back(&player);
    }
    return result;
}

std::vector<Player *> SessionManager::getAllPlayers() {
    std::lock_guard<std::mutex> lock(_playersMutex);

    std::vector<Player *> result;
    for (auto &[id, player] : _players)
        result.push_back(&player);
    return result;
}

size_t SessionManager::getPlayerCount() const {
    std::lock_guard<std::mutex> lock(_playersMutex);
    return _players.size();
}

void SessionManager::sendTcp(uint32_t playerId, const MessageData &data,
                             Priority priority) {
    std::lock_guard<std::mutex> lock(_playersMutex);

    auto it = _players.find(playerId);
    if (it != _players.end() && it->second.tcpFd >= 0) {
        _tcpServer.send(it->second.tcpFd, data, priority);
    }
}

void SessionManager::sendTcp(uint32_t playerId, const PreparedMessage &msg) {
    sendTcp(playerId, msg.data, msg.priority);
}

void SessionManager::sendUdp(uint32_t playerId, const MessageData &data) {
    if (!_config.enableUdp || !_udpServer) {
        return;
    }

    std::lock_guard<std::mutex> lock(_playersMutex);

    auto it = _players.find(playerId);
    if (it != _players.end() && it->second.udpLinked) {
        _udpServer->send(data, it->second.udpAddr);
    }
}

void SessionManager::sendUdp(uint32_t playerId, const PreparedMessage &msg) {
    sendUdp(playerId, msg.data);
}

void SessionManager::broadcastTcp(const MessageData &data, uint8_t roomId) {
    if (roomId == 0) {
        _tcpServer.send(data);
    } else {
        std::lock_guard<std::mutex> lock(_playersMutex);
        for (auto &[id, player] : _players) {
            if (player.roomId == roomId && player.tcpFd >= 0) {
                _tcpServer.send(player.tcpFd, data);
            }
        }
    }
}

void SessionManager::broadcastTcp(const PreparedMessage &msg, uint8_t roomId) {
    broadcastTcp(msg.data, roomId);
}

void SessionManager::broadcastUdp(const MessageData &data, uint8_t roomId) {
    if (!_config.enableUdp || !_udpServer) {
        return;
    }

    if (roomId == 0) {
        _udpServer->send(data);
    } else {
        std::lock_guard<std::mutex> lock(_playersMutex);
        for (auto &[id, player] : _players) {
            if (player.roomId == roomId && player.udpLinked) {
                _udpServer->send(data, player.udpAddr);
            }
        }
    }
}

void SessionManager::broadcastUdp(const PreparedMessage &msg, uint8_t roomId) {
    broadcastUdp(msg.data, roomId);
}

std::optional<DecodedMessage> SessionManager::popMessage(Priority priority) {
    return _queue.pop(priority);
}

bool SessionManager::hasMessages() const { return !_queue.isEmpty(); }

bool SessionManager::compareUdpAddr(const sockaddr_in &a,
                                    const sockaddr_in &b) const {
    return a.sin_addr.s_addr == b.sin_addr.s_addr && a.sin_port == b.sin_port;
}
