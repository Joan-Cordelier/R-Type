/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Room implementation
*/

#include "Room.hpp"
#include "../Gameplay/RoomGameHandler.hpp"
#include "../Logs/Logger.hpp"

#include <cstring>

Room::Room(uint8_t id, const std::string &name) : _id(id), _name(name) {
    LOG_INFO("Room " + std::to_string(id) + " '" + name + "' created");
}

Room::~Room() {
    stop();
    LOG_INFO("Room " + std::to_string(_id) + " destroyed");
}

void Room::start() {
    if (_running) {
        LOG_WARN("Room " + std::to_string(_id) + " already running");
        return;
    }

    // Create session with dynamic ports for room
    _session = std::make_unique<SessionManager>(SessionConfig::room());
    _session->start();

    LOG_INFO("Room " + std::to_string(_id) +
             " network started (TCP:" + std::to_string(getTcpPort()) +
             ", UDP:" + std::to_string(getUdpPort()) + ")");

    // Create game handler with send/broadcast callbacks
    RoomGameHandler::SendCallback sendUdp = [this](uint32_t playerId,
                                                   const PreparedMessage &msg) {
        _session->sendUdp(playerId, msg);
    };
    RoomGameHandler::SendCallback sendTcp = [this](uint32_t playerId,
                                                   const PreparedMessage &msg) {
        _session->sendTcp(playerId, msg);
    };
    RoomGameHandler::BroadcastCallback broadcastUdp =
        [this](const PreparedMessage &msg) { _session->broadcastUdp(msg); };
    RoomGameHandler::BroadcastCallback broadcastTcp =
        [this](const PreparedMessage &msg) { _session->broadcastTcp(msg); };

    _game = std::make_unique<RoomGameHandler>(_running, sendUdp, sendTcp,
                                              broadcastUdp, broadcastTcp);

    _running = true;
    _gameStarted = true;

    // Start room thread (handles network messages + game loop)
    _roomThread = std::thread(&Room::runLoop, this);

    LOG_INFO("Room " + std::to_string(_id) + " started on dedicated thread");
}

void Room::stop() {
    if (!_running) {
        return;
    }

    _running = false;

    if (_roomThread.joinable()) {
        _roomThread.join();
    }

    _game.reset();

    if (_session) {
        _session->stop();
        _session.reset();
    }

    LOG_INFO("Room " + std::to_string(_id) + " stopped");
}

uint16_t Room::getTcpPort() const {
    return _session ? _session->getTcpPort() : 0;
}

uint16_t Room::getUdpPort() const {
    return _session ? _session->getUdpPort() : 0;
}

void Room::runLoop() {
    auto lastTime = std::chrono::steady_clock::now();
    constexpr float targetFrameTime = 1.0f / 30.0f; // 30 ticks per second

    while (_running) {
        auto currentTime = std::chrono::steady_clock::now();
        float deltaTime =
            std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Process network messages
        processMessages();

        // Update game (runs on same thread now - no mutex needed)
        if (_game) {
            _game->updateGame(deltaTime);
            _game->sendUpdatedPositionToAllPlayers();
        }

        // Frame rate limiting
        auto frameEnd = std::chrono::steady_clock::now();
        float frameTime =
            std::chrono::duration<float>(frameEnd - currentTime).count();
        if (frameTime < targetFrameTime) {
            std::this_thread::sleep_for(
                std::chrono::duration<float>(targetFrameTime - frameTime));
        }
    }

    LOG_INFO("Room " + std::to_string(_id) + " game loop stopped");
}

void Room::processMessages() {
    // Process all pending messages
    for (Priority prio : {Priority::CRITICAL, Priority::HIGH, Priority::MEDIUM,
                          Priority::LOW}) {
        while (true) {
            auto msg = _session->popMessage(prio);
            if (!msg.has_value())
                break;
            LOG_DEBUG(
                "Room " + std::to_string(_id) +
                ": Processing message OpCode=" + std::to_string(msg->opCode) +
                " Len=" + std::to_string(msg->len));
            handleMessage(*msg);
        }
    }
}

void Room::handleMessage(const DecodedMessage &msg) {
    // Resolve playerId if needed
    uint32_t playerId = msg.playerId;
    if (playerId == 0) {
        if (msg.tcpFd >= 0) {
            auto *player = _session->getPlayerByTcpFd(msg.tcpFd);
            if (player)
                playerId = player->id;
        } else if (msg.udpAddr.sin_port != 0) {
            auto *player = _session->getPlayerByUdpAddr(msg.udpAddr);
            if (player)
                playerId = player->id;
        }
    }

    switch (msg.opCode) {
    case OpCode::ROOM_READY: {
        // Player connected to room and is ready
        // First 4 bytes of data contain the real PlayerID (from Lobby)
        if (msg.data.size() >= 4) {
            uint32_t realPlayerId = (static_cast<uint32_t>(msg.data[0]) << 24) |
                                    (static_cast<uint32_t>(msg.data[1]) << 16) |
                                    (static_cast<uint32_t>(msg.data[2]) << 8) |
                                    static_cast<uint32_t>(msg.data[3]);

            // If the current playerId (assigned by Room Session) differs from
            // realPlayerId
            if (playerId != 0 && playerId != realPlayerId && _session) {
                LOG_INFO("Room " + std::to_string(_id) +
                         ": Remapping cached ID " + std::to_string(playerId) +
                         " to real ID " + std::to_string(realPlayerId));

                // Update SessionManager mapping
                _session->updatePlayerId(playerId, realPlayerId);
                playerId = realPlayerId; // Update local var for game join
            }

            if (_game) {
                _game->onPlayerConnect(playerId);
                LOG_INFO("Room " + std::to_string(_id) + ": Player " +
                         std::to_string(playerId) + " ready");
            }
        }
        break;
    }
    case OpCode::LINK: {
        if (msg.data.size() >= 4) {
            uint32_t linkPlayerId = (static_cast<uint32_t>(msg.data[0]) << 24) |
                                    (static_cast<uint32_t>(msg.data[1]) << 16) |
                                    (static_cast<uint32_t>(msg.data[2]) << 8) |
                                    static_cast<uint32_t>(msg.data[3]);

            _session->linkPlayerUdp(linkPlayerId, msg.udpAddr);

            if (_game) {
                _game->onPlayerLinked(linkPlayerId);
            }
            LOG_INFO("Room " + std::to_string(_id) + ": Player " +
                     std::to_string(linkPlayerId) + " UDP linked");
        }
        break;
    }
    case OpCode::MOVE_INPUT: {
        if (msg.data.size() >= 12 && playerId != 0 && _game) {
            float vx, vy;
            std::memcpy(&vx, &msg.data[4], sizeof(float));
            std::memcpy(&vy, &msg.data[8], sizeof(float));

            RoomMoveData moveData{playerId, vx, vy};
            _game->onPlayerMove(moveData);
        }
        break;
    }
    case OpCode::SHOOT: {
        if (playerId != 0 && _game) {
            RoomShootData shootData{playerId};
            _game->onPlayerShoot(shootData);
        }
        break;
    }
    case OpCode::ROOM_LEAVE: {
        if (playerId != 0) {
            LOG_INFO("Room " + std::to_string(_id) + ": Player " +
                     std::to_string(playerId) + " requested to leave");
            removePlayer(playerId);
        }
        break;
    }
    default:
        LOG_DEBUG("Room " + std::to_string(_id) + ": Unhandled OpCode " +
                  std::to_string(msg.opCode));
        break;
    }
}

void Room::addPlayer(uint32_t playerId, int tcpFd) {
    (void)tcpFd; // Not used - room's SessionManager handles connection tracking

    {
        std::lock_guard<std::mutex> lock(_playersMutex);
        _playerIds.insert(playerId);
    }

    // Add to session's player tracking
    // Note: For room connections, players are added via SessionManager
    // callbacks This is for explicit tracking if needed
    LOG_INFO("Room " + std::to_string(_id) + ": Player " +
             std::to_string(playerId) + " added");
}

void Room::removePlayer(uint32_t playerId) {
    {
        std::lock_guard<std::mutex> lock(_playersMutex);
        _playerIds.erase(playerId);
    }

    if (_game) {
        _game->onPlayerDisconnect(playerId);
    }

    if (_session) {
        _session->removePlayer(playerId);
    }

    LOG_INFO("Room " + std::to_string(_id) + ": Player " +
             std::to_string(playerId) + " removed");
}

void Room::linkPlayerUdp(uint32_t playerId, const sockaddr_in &addr) {
    if (_session) {
        _session->linkPlayerUdp(playerId, addr);
    }
}

bool Room::hasPlayer(uint32_t playerId) const {
    std::lock_guard<std::mutex> lock(_playersMutex);
    return _playerIds.count(playerId) > 0;
}

std::set<uint32_t> Room::getPlayerIds() const {
    std::lock_guard<std::mutex> lock(_playersMutex);
    return _playerIds;
}

size_t Room::getPlayerCount() const {
    std::lock_guard<std::mutex> lock(_playersMutex);
    return _playerIds.size();
}
