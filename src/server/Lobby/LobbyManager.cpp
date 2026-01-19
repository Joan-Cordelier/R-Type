/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LobbyManager
*/

#include "LobbyManager.hpp"
#include "../../common/Data/MessageFactory.hpp"
#include "../../common/Data/RoomConfig.hpp"
#include "../Database/UserDatabase.hpp"
#include "../Logs/Logger.hpp"
#include <chrono>
#include <tuple>

LobbyManager::LobbyManager(const std::string &configPath, PrometheusExporter &monitor)
    : _session(monitor), _configPath(configPath), _monitor(monitor),
      _lastCleanupCheck(std::chrono::steady_clock::now()) {
    _session.setOnPlayerDisconnect([this](const Player &player) { onPlayerDisconnect(player); });
}

LobbyManager::~LobbyManager() {
    stop();
}

void LobbyManager::run() {
    _session.start();
    LOG_INFO("LobbyManager started");

    while (_running) {
        processMessages();

        // Check for empty rooms to cleanup every second
        auto now = std::chrono::steady_clock::now();
        if (now - _lastCleanupCheck >= std::chrono::seconds(1)) {
            cleanupEmptyRooms();
            _lastCleanupCheck = now;
        }

        // Sleep to prevent CPU hogging
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    _session.stop();
    LOG_INFO("LobbyManager stopped");
}

void LobbyManager::stop() {
    _running = false;
    for (auto &[id, room] : _rooms) {
        room->stop();
    }
    _rooms.clear();
}

void LobbyManager::processMessages() {
    auto critical = _session.popMessage(Priority::CRITICAL);
    if (critical.has_value()) {
        dispatchMessage(critical.value());
        return; // Prioritize critical
    }

    auto high = _session.popMessage(Priority::HIGH);
    if (high.has_value()) {
        dispatchMessage(high.value());
        return;
    }

    auto medium = _session.popMessage(Priority::MEDIUM);
    if (medium.has_value()) {
        dispatchMessage(medium.value());
        return;
    }

    auto low = _session.popMessage(Priority::LOW);
    if (low.has_value()) {
        dispatchMessage(low.value());
    }
}

void LobbyManager::dispatchMessage(DecodedMessage &msg) {
    std::string opName = MessageFactory::getInstance().getOpCodeName(msg.opCode);
    _monitor.recordPacketType("Lobby_" + opName);
    if (msg.playerId == 0 && msg.tcpFd > 0) {
        auto player = _session.getPlayerByTcpFd(msg.tcpFd);
        if (player)
            msg.playerId = player->id;
    }

    switch (msg.opCode) {
    case REGISTER:
        handleRegister(msg);
        return;
    case LOGIN:
        handleLogin(msg);
        return;
    case GUEST_LOGIN:
        handleGuestLogin(msg);
        return;
    case CREATE_ROOM:
        handleCreateRoom(msg);
        return;
    case JOIN_ROOM:
        handleJoinRoom(msg);
        return;
    case LIST_ROOMS:
        handleListRooms(msg);
        return;
    case CONNECT:
        handleConnect(msg);
        return;
    case LINK:
        handleLink(msg);
        return;
    case CHAT_MESSAGE:
        handleChatMessage(msg);
        return;
    case SCOREBOARD_REQUEST:
        handleScoreboardRequest(msg);
        return;
    case DISCONNECT:
        handleDisconnect(msg);
        return;
    case GET_LEADERBOARD:
        handleGetLeaderboard(msg);
        return;
    default:
        break;
    }

    // Dispatch to Room
    if (msg.playerId != 0) {
        auto player = _session.getPlayer(msg.playerId);
        if (player && player->roomId != 0) {
            auto it = _rooms.find(player->roomId);
            if (it != _rooms.end()) {
                it->second->pushMessage(msg);
                return;
            }
        }
    } else {
        // Try to find player by UDP/TCP if ID is missing but connection info exists
        if (msg.udpAddr.sin_port != 0) {
            auto player = _session.getPlayerByUdpAddr(msg.udpAddr);
            if (player) {
                msg.playerId = player->id;
                // Try to re-dispatch with valid ID
                if (player->roomId != 0) {
                    auto it = _rooms.find(player->roomId);
                    if (it != _rooms.end()) {
                        it->second->pushMessage(msg);
                        return;
                    }
                }
            }
        }
    }

    // If we reach here for game messages, player might not be in a room or
    // invalid.
    if (msg.opCode != PARSING_ERROR && msg.opCode != INCOMPLETE) {
        // LOG_WARN("Message " + std::to_string(msg.opCode) + " from player " +
        // std::to_string(msg.playerId) + " not handled (not in room?)");
    }
}

void LobbyManager::handleCreateRoom(const DecodedMessage &msg) {
    // Parse room configuration from message
    RoomConfig config;
    if (msg.data.size() >= 3) {
        config.maxPlayers = msg.data[0];
        config.gameMode = static_cast<GameMode>(msg.data[1]);
        config.difficulty = static_cast<Difficulty>(msg.data[2]);

        // Validate maxPlayers (1-4)
        if (config.maxPlayers < 1)
            config.maxPlayers = 1;
        if (config.maxPlayers > 4)
            config.maxPlayers = 4;
    }

    uint32_t roomId = _nextRoomId++;
    auto room = std::make_shared<Room>(roomId, _session, _configPath, _monitor, config);
    _rooms[roomId] = room;
    room->start();

    LOG_INFO("Room " + std::to_string(roomId) + " created by player " +
             std::to_string(msg.playerId) + " (MaxPlayers: " + std::to_string(config.maxPlayers) +
             ", Mode: " + config.getGameModeStr() + ", Difficulty: " + config.getDifficultyStr() +
             ")");

    auto &factory = MessageFactory::getInstance();
    MessageData payload = factory.encodeMessageRoomCreated(roomId);
    PreparedMessage response = factory.createMessage(ROOM_CREATED, payload);
    _session.sendTcp(msg.playerId, response);

    // Auto-join creator?
    // Let's assume client sends JOIN_ROOM after receiving ROOM_CREATED, or we
    // auto-join. Spec didn't say. Let's wait for explicit JOIN.
}

void LobbyManager::handleJoinRoom(const DecodedMessage &msg) {
    if (msg.data.size() < 4)
        return;

    uint32_t roomId =
        (static_cast<uint32_t>(msg.data[0]) << 24) | (static_cast<uint32_t>(msg.data[1]) << 16) |
        (static_cast<uint32_t>(msg.data[2]) << 8) | static_cast<uint32_t>(msg.data[3]);

    auto it = _rooms.find(roomId);
    bool success = false;

    if (it != _rooms.end()) {
        if (it->second->isPlayerBanned(msg.playerId)) {
            LOG_INFO("Player " + std::to_string(msg.playerId) + " denied entry to room " +
                     std::to_string(roomId) + " (banned - no come back policy)");
        } else if (!it->second->isFull()) {
            auto player = _session.getPlayer(msg.playerId);
            if (player) {
                // Leave old room if any
                if (player->roomId != 0) {
                    auto oldRoomIt = _rooms.find(player->roomId);
                    if (oldRoomIt != _rooms.end()) {
                        oldRoomIt->second->removePlayer(msg.playerId);
                    }
                }

                player->roomId = roomId; // Note: roomId is uint8_t in Player struct...
                                         // Room ID is uint32_t.
                // Issue: Player struct has uint8_t roomId. New system uses uint32_t.
                // I need to update Player struct to uint32_t or cast.
                // Assuming Room IDs fit in uint8 for now (255 rooms max).

                it->second->addPlayer(msg.playerId);
                success = true;
                LOG_INFO("Player " + std::to_string(msg.playerId) + " joined room " +
                         std::to_string(roomId));
            }
        }
    }

    auto &factory = MessageFactory::getInstance();
    MessageData payload = factory.encodeMessageJoinAck(roomId, success);
    PreparedMessage response = factory.createMessage(JOIN_ACK, payload);
    _session.sendTcp(msg.playerId, response);

    if (success) {
        std::vector<uint8_t> dummy;
        DecodedMessage connectMsg;
        connectMsg.opCode = CONNECT;
        connectMsg.playerId = msg.playerId;
        connectMsg.tcpFd = -1; // Not needed if playerId is set
        connectMsg.priority = Priority::CRITICAL;
        it->second->pushMessage(connectMsg);
    }
}

void LobbyManager::handleListRooms(const DecodedMessage &msg) {
    std::vector<std::tuple<uint32_t, uint8_t, uint8_t>> roomList; // roomId, playerCount, maxPlayers
    for (const auto &[id, room] : _rooms) {
        roomList.push_back(
            {id, static_cast<uint8_t>(room->getPlayerCount()), room->getConfig().maxPlayers});
    }

    auto &factory = MessageFactory::getInstance();
    MessageData payload = factory.encodeMessageRoomList(roomList);
    PreparedMessage response = factory.createMessage(ROOM_LIST, payload);
    LOG_DEBUG("ROOM_LIST sent (" + std::to_string(response.data.size()) + " bytes)");
    _session.sendTcp(msg.playerId, response);
}

void LobbyManager::handleConnect(const DecodedMessage &msg) {
    // Just ack connection, don't auto-join room 0 anymore.
    // Or auto-join lobby "room"?
    // For now, just CONNECT_ACK to say "Welcome to Lobby".

    if (msg.tcpFd < 0)
        return;
    auto player = _session.getPlayerByTcpFd(msg.tcpFd);
    if (!player)
        return;

    player->connected = true;
    uint32_t playerId = player->id;

    auto &factory = MessageFactory::getInstance();
    std::vector<uint8_t> payload;
    payload.push_back(static_cast<uint8_t>((playerId >> 24) & 0xFF));
    payload.push_back(static_cast<uint8_t>((playerId >> 16) & 0xFF));
    payload.push_back(static_cast<uint8_t>((playerId >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(playerId & 0xFF));

    PreparedMessage response = factory.createMessage(CONNECT_ACK, payload);
    _session.sendTcp(playerId, response);

    LOG_INFO("Player " + std::to_string(playerId) + " connected to Lobby");
}

void LobbyManager::onPlayerDisconnect(const Player &player) {
    // Remove from room if in one
    if (player.roomId != 0) {
        auto it = _rooms.find(player.roomId);
        if (it != _rooms.end()) {
            it->second->removePlayer(player.id, player.userId, player.username);
        }
    }
}

void LobbyManager::handleDisconnect(const DecodedMessage &msg) {
    // Player wants to leave their current room (Exit Party)
    if (msg.playerId == 0) {
        LOG_WARN("DISCONNECT message with no playerId");
        return;
    }

    auto player = _session.getPlayer(msg.playerId);
    if (!player) {
        LOG_WARN("DISCONNECT from unknown player " + std::to_string(msg.playerId));
        return;
    }

    if (player->roomId != 0) {
        auto it = _rooms.find(player->roomId);
        if (it != _rooms.end()) {
            it->second->removePlayer(player->id, player->userId, player->username);
            LOG_INFO("Player " + std::to_string(player->id) + " left room " +
                     std::to_string(player->roomId) + " (Exit Party)");
        }
        player->roomId = 0; // Clear room assignment
    }
}

void LobbyManager::handleLink(const DecodedMessage &msg) {
    if (msg.data.size() < 4)
        return;

    uint32_t playerId =
        (static_cast<uint32_t>(msg.data[0]) << 24) | (static_cast<uint32_t>(msg.data[1]) << 16) |
        (static_cast<uint32_t>(msg.data[2]) << 8) | static_cast<uint32_t>(msg.data[3]);

    // Security check: ensure the playerId claimed matches the sender?
    // Since UDP is connectionless, we trust the payload ID for association.
    // Validation: check if player exists.
    Player *player = _session.getPlayer(playerId);
    if (player) {
        _session.linkPlayerUdp(playerId, msg.udpAddr);
        LOG_INFO("Linked UDP address for player " + std::to_string(playerId));
    } else {
        LOG_WARN("LINK request for unknown player " + std::to_string(playerId));
    }
}

void LobbyManager::cleanupEmptyRooms() {
    std::vector<uint32_t> roomsToRemove;

    // Find rooms that have been empty for too long OR have completed their game
    for (const auto &[id, room] : _rooms) {
        if (room->isGameComplete() || room->hasBeenEmptyFor(EMPTY_ROOM_TIMEOUT)) {
            roomsToRemove.push_back(id);
        }
    }

    // Remove rooms
    for (uint32_t id : roomsToRemove) {
        auto it = _rooms.find(id);
        if (it != _rooms.end()) {
            bool wasComplete = it->second->isGameComplete();
            it->second->stop();
            _rooms.erase(it);
            if (wasComplete) {
                LOG_INFO("Room " + std::to_string(id) + " closed (game complete - victory)");
            } else {
                LOG_INFO("Room " + std::to_string(id) + " auto-closed (empty for 5 seconds)");
            }
        }
    }
}

// ==================== Authentication Handlers ====================

void LobbyManager::handleRegister(const DecodedMessage &msg) {
    if (msg.data.size() < 2) {
        LOG_WARN("Invalid REGISTER message: too short");
        return;
    }

    auto &factory = MessageFactory::getInstance();

    // Parse username
    size_t offset = 0;
    uint8_t usernameLen = msg.data[offset++];
    if (offset + usernameLen > msg.data.size())
        return;
    std::string username(msg.data.begin() + offset, msg.data.begin() + offset + usernameLen);
    offset += usernameLen;

    // Parse password
    if (offset >= msg.data.size())
        return;
    uint8_t passwordLen = msg.data[offset++];
    if (offset + passwordLen > msg.data.size())
        return;
    std::string password(msg.data.begin() + offset, msg.data.begin() + offset + passwordLen);

    LOG_INFO("Registration attempt: " + username);

    // Try to register
    auto &userDb = UserDatabase::getInstance();
    uint32_t userId = userDb.registerUser(username, password);

    MessageData payload;
    if (userId > 0) {
        payload = factory.encodeMessageRegisterAck(true, userId, "");
        LOG_INFO("Registration successful: " + username + " (ID: " + std::to_string(userId) + ")");
    } else {
        payload = factory.encodeMessageRegisterAck(false, 0, "Username taken or invalid");
        LOG_WARN("Registration failed: " + username);
    }

    PreparedMessage response = factory.createMessage(REGISTER_ACK, payload);

    // Send to the TCP connection that sent the message
    if (msg.tcpFd > 0) {
        auto player = _session.getPlayerByTcpFd(msg.tcpFd);
        if (player) {
            _session.sendTcp(player->id, response);
        }
    }
}

void LobbyManager::handleLogin(const DecodedMessage &msg) {
    if (msg.data.size() < 2) {
        LOG_WARN("Invalid LOGIN message: too short");
        return;
    }

    auto &factory = MessageFactory::getInstance();

    // Parse username
    size_t offset = 0;
    uint8_t usernameLen = msg.data[offset++];
    if (offset + usernameLen > msg.data.size())
        return;
    std::string username(msg.data.begin() + offset, msg.data.begin() + offset + usernameLen);
    offset += usernameLen;

    // Parse password
    if (offset >= msg.data.size())
        return;
    uint8_t passwordLen = msg.data[offset++];
    if (offset + passwordLen > msg.data.size())
        return;
    std::string password(msg.data.begin() + offset, msg.data.begin() + offset + passwordLen);

    LOG_INFO("Login attempt: " + username);

    // Try to login
    auto &userDb = UserDatabase::getInstance();
    auto userOpt = userDb.login(username, password);

    MessageData payload;
    if (userOpt.has_value()) {
        auto &user = userOpt.value();

        // Check if user is banned
        if (userDb.isUserBanned(user.id)) {
            payload =
                factory.encodeMessageLoginAck(false, 0, "", "You are banned from this server");
            LOG_WARN("Login rejected (banned): " + username + " (ID: " + std::to_string(user.id) +
                     ")");
        } else {
            payload = factory.encodeMessageLoginAck(true, user.id, user.username, "");

            // Update the player's username in session
            if (msg.tcpFd > 0) {
                auto player = _session.getPlayerByTcpFd(msg.tcpFd);
                if (player) {
                    player->username = user.username;
                    player->userId = user.id;
                    player->isGuest = false;
                }
            }

            LOG_INFO("Login successful: " + username + " (ID: " + std::to_string(user.id) + ")");
        }
    } else {
        payload = factory.encodeMessageLoginAck(false, 0, "", "Invalid username or password");
        LOG_WARN("Login failed: " + username);
    }

    PreparedMessage response = factory.createMessage(LOGIN_ACK, payload);

    if (msg.tcpFd > 0) {
        auto player = _session.getPlayerByTcpFd(msg.tcpFd);
        if (player) {
            _session.sendTcp(player->id, response);
        }
    }
}

void LobbyManager::handleGuestLogin(const DecodedMessage &msg) {
    auto &factory = MessageFactory::getInstance();
    auto &userDb = UserDatabase::getInstance();

    // Create guest user
    User guest = userDb.createGuest();

    LOG_INFO("Guest login: " + guest.username + " (ID: " + std::to_string(guest.id) + ")");

    // Update the player's info in session
    if (msg.tcpFd > 0) {
        auto player = _session.getPlayerByTcpFd(msg.tcpFd);
        if (player) {
            player->username = guest.username;
            player->userId = guest.id;
            player->isGuest = true;
        }
    }

    MessageData payload = factory.encodeMessageGuestLoginAck(guest.id, guest.username);
    PreparedMessage response = factory.createMessage(GUEST_LOGIN_ACK, payload);

    if (msg.tcpFd > 0) {
        auto player = _session.getPlayerByTcpFd(msg.tcpFd);
        if (player) {
            _session.sendTcp(player->id, response);
        }
    }
}

void LobbyManager::handleChatMessage(const DecodedMessage &msg) {
    auto &factory = MessageFactory::getInstance();

    // Get the sender player
    Player *sender = nullptr;
    if (msg.playerId != 0) {
        sender = _session.getPlayer(msg.playerId);
    } else if (msg.tcpFd > 0) {
        sender = _session.getPlayerByTcpFd(msg.tcpFd);
    }

    if (!sender) {
        LOG_WARN("CHAT_MESSAGE from unknown player");
        return;
    }

    // Parse the chat message
    if (msg.data.empty()) {
        return;
    }

    uint8_t msgLen = msg.data[0];
    if (msg.data.size() < static_cast<size_t>(1 + msgLen)) {
        return;
    }

    std::string chatMessage;
    for (size_t i = 0; i < msgLen; ++i) {
        chatMessage += static_cast<char>(msg.data[1 + i]);
    }
    if (chatMessage.empty()) {
        return;
    }

    LOG_INFO("Chat from " + sender->username + ": " + chatMessage);

    // If sender is in a room, broadcast to room members
    // Otherwise, they're in the lobby - broadcast to all lobby players
    MessageData payload =
        factory.encodeMessageChatBroadcast(sender->userId, sender->username, chatMessage);
    PreparedMessage broadcast = factory.createMessage(CHAT_BROADCAST, payload);

    if (sender->roomId != 0) {
        // Broadcast to room members
        auto it = _rooms.find(sender->roomId);
        if (it != _rooms.end()) {
            for (uint32_t playerId : it->second->getPlayers()) {
                _session.sendTcp(playerId, broadcast);
            }
        }
    } else {
        // Broadcast to all players in lobby (no room)
        for (Player *player : _session.getAllPlayers()) {
            if (player->roomId == 0) {
                _session.sendTcp(player->id, broadcast);
            }
        }
    }
}

void LobbyManager::handleScoreboardRequest(const DecodedMessage &msg) {
    auto &factory = MessageFactory::getInstance();
    auto &userDb = UserDatabase::getInstance();

    // Get player to respond to
    uint32_t playerId = msg.playerId;
    if (playerId == 0 && msg.tcpFd > 0) {
        auto player = _session.getPlayerByTcpFd(msg.tcpFd);
        if (player) {
            playerId = player->id;
        }
    }

    if (playerId == 0) {
        LOG_WARN("SCOREBOARD_REQUEST from unknown player");
        return;
    }

    // Get top 10 scores
    auto topScores = userDb.getTopScores(10);

    LOG_INFO("Sending scoreboard with " + std::to_string(topScores.size()) + " entries to player " +
             std::to_string(playerId));

    MessageData payload = factory.encodeMessageScoreboardResponse(topScores);
    PreparedMessage response = factory.createMessage(SCOREBOARD_RESPONSE, payload);
    _session.sendTcp(playerId, response);
}

void LobbyManager::handleGetLeaderboard(const DecodedMessage &msg) {
    auto &factory = MessageFactory::getInstance();
    auto &userDb = UserDatabase::getInstance();

    // Get player to respond to
    uint32_t playerId = msg.playerId;
    if (playerId == 0 && msg.tcpFd > 0) {
        auto player = _session.getPlayerByTcpFd(msg.tcpFd);
        if (player) {
            playerId = player->id;
        }
    }

    if (playerId == 0) {
        LOG_WARN("GET_LEADERBOARD from unknown player");
        return;
    }

    // Check if player is registered (guests can't view leaderboard)
    auto player = _session.getPlayer(playerId);
    if (!player || player->userId == 0) {
        LOG_WARN("GET_LEADERBOARD from guest user - denied");
        return;
    }

    // Get difficulty from message
    uint8_t difficulty = 0;
    if (!msg.data.empty()) {
        difficulty = msg.data[0];
    }

    // Get top 10 endless scores for this difficulty
    auto topScores = userDb.getEndlessTopScores(difficulty, 10);

    LOG_INFO("Sending endless leaderboard (difficulty " + std::to_string(difficulty) + ") with " +
             std::to_string(topScores.size()) + " entries to player " + std::to_string(playerId));

    MessageData payload = factory.encodeMessageLeaderboardData(difficulty, topScores);
    PreparedMessage response = factory.createMessage(LEADERBOARD_DATA, payload);
    _session.sendTcp(playerId, response);
}

// Admin Console Methods

std::vector<LobbyManager::RoomInfo> LobbyManager::getAdminRoomList() {
    std::vector<RoomInfo> result;
    for (const auto &[id, room] : _rooms) {
        RoomInfo info;
        info.id = id;
        info.playerCount = room->getPlayerCount();
        info.gameMode = RoomConfig::gameModeToString(room->getGameMode());
        info.difficulty = RoomConfig::difficultyToString(room->getDifficulty());
        result.push_back(info);
    }
    return result;
}

std::vector<LobbyManager::UserInfo> LobbyManager::getAdminUserList() {
    std::vector<UserInfo> result;
    auto players = _session.getPlayerSessions();
    for (const auto &[playerId, player] : players) {
        UserInfo info;
        info.playerId = playerId;
        info.userId = player.userId;
        info.username = player.username;
        info.roomId = player.roomId;
        info.isGuest = (player.userId == 0);
        result.push_back(info);
    }
    return result;
}

bool LobbyManager::kickUser(uint32_t playerId) {
    auto *player = _session.getPlayer(playerId);
    if (!player) {
        return false;
    }

    LOG_INFO("Admin kicked player " + std::to_string(playerId) + " (" + player->username + ")");

    // Remove from room if in one
    if (player->roomId > 0) {
        auto it = _rooms.find(player->roomId);
        if (it != _rooms.end()) {
            it->second->removePlayer(playerId, player->userId, player->username);
        }
    }

    // Disconnect the player
    _session.disconnectPlayer(playerId);
    return true;
}

bool LobbyManager::banUser(uint32_t playerId) {
    auto *player = _session.getPlayer(playerId);
    if (!player) {
        return false;
    }

    LOG_INFO("Admin banned player " + std::to_string(playerId) + " (" + player->username + ")");

    // Add to ban list (if registered user)
    if (player->userId > 0) {
        auto &userDb = UserDatabase::getInstance();
        userDb.banUser(player->userId);
    }

    // Kick them after banning
    return kickUser(playerId);
}

bool LobbyManager::unbanUser(uint32_t userId) {
    auto &userDb = UserDatabase::getInstance();
    return userDb.unbanUser(userId);
}

size_t LobbyManager::getTotalConnections() const {
    return _session.getTotalConnections();
}

std::vector<LobbyManager::UserInfo> LobbyManager::getBannedUsers() {
    auto &userDb = UserDatabase::getInstance();
    auto bannedIds = userDb.getBannedUserIds();
    std::vector<UserInfo> result;
    for (uint32_t userId : bannedIds) {
        UserInfo info;
        info.playerId = 0; // Not connected
        info.userId = userId;
        info.username = userDb.getUsername(userId);
        info.roomId = 0;
        info.isGuest = false;
        result.push_back(info);
    }
    return result;
}
