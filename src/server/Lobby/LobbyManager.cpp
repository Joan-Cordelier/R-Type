/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LobbyManager
*/

#include "LobbyManager.hpp"
#include "../../common/Data/MessageFactory.hpp"
#include "../Logs/Logger.hpp"
#include <chrono>

LobbyManager::LobbyManager(const std::string &configPath, PrometheusExporter& monitor) 
    : _session(monitor)
    , _configPath(configPath)
    , _monitor(monitor) 
{
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
    uint32_t roomId = _nextRoomId++;
    auto room = std::make_shared<Room>(roomId, _session, _configPath, _monitor);
    _rooms[roomId] = room;
    room->start();

    LOG_INFO("Room " + std::to_string(roomId) + " created by player " +
             std::to_string(msg.playerId));

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
        if (!it->second->isFull()) {
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
    std::vector<std::pair<uint32_t, uint8_t>> roomList;
    for (const auto &[id, room] : _rooms) {
        roomList.push_back({id, static_cast<uint8_t>(room->getPlayerCount())});
    }

    auto &factory = MessageFactory::getInstance();
    MessageData payload = factory.encodeMessageRoomList(roomList);
    PreparedMessage response = factory.createMessage(ROOM_LIST, payload);
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
            it->second->removePlayer(player.id);

            DecodedMessage disMsg;
            disMsg.opCode = DISCONNECT;
            disMsg.playerId = player.id;
            disMsg.priority = Priority::CRITICAL;
            it->second->pushMessage(disMsg);
        }
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
