/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LobbyHandler implementation
*/

#include "LobbyHandler.hpp"
#include "../Logs/Logger.hpp"

LobbyHandler::LobbyHandler(SessionManager &session, RoomManager &roomManager)
    : _session(session), _roomManager(roomManager) {}

void LobbyHandler::handleMessage(const DecodedMessage &msg) {
    uint32_t playerId = msg.playerId;

    // Resolve playerId from TCP fd if not set (for initial connection)
    if (playerId == 0 && msg.tcpFd >= 0) {
        auto *player = _session.getPlayerByTcpFd(msg.tcpFd);
        if (player) {
            playerId = player->id;
        }
    }

    switch (msg.opCode) {
    case OpCode::CONNECT:
        if (playerId != 0)
            handleConnect(playerId);
        break;
    case OpCode::ROOM_LIST:
        if (playerId != 0)
            handleRoomList(playerId);
        break;
    case OpCode::ROOM_CREATE:
        if (playerId != 0)
            handleRoomCreate(playerId, msg.data);
        break;
    case OpCode::ROOM_JOIN:
        if (playerId != 0)
            handleRoomJoin(playerId, msg.data);
        break;
    case OpCode::ROOM_LEAVE:
        if (playerId != 0)
            handleRoomLeave(playerId);
        break;
    default:
        // Ignore game-specific messages in lobby
        break;
    }
}

void LobbyHandler::handleConnect(uint32_t playerId) {
    auto *player = _session.getPlayer(playerId);
    if (!player)
        return;

    player->connected = true;

    // Send CONNECT_ACK with player ID
    auto &factory = MessageFactory::getInstance();
    std::vector<uint8_t> payload;
    payload.push_back(static_cast<uint8_t>((playerId >> 24) & 0xFF));
    payload.push_back(static_cast<uint8_t>((playerId >> 16) & 0xFF));
    payload.push_back(static_cast<uint8_t>((playerId >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(playerId & 0xFF));
    _session.sendTcp(playerId,
                     factory.createMessage(OpCode::CONNECT_ACK, payload));

    LOG_INFO("Lobby: Player " + std::to_string(playerId) + " connected");

    // Auto-join default room
    Room *defRoom = _roomManager.getDefaultRoom();
    if (defRoom) {
        std::vector<uint8_t> joinPayload;
        joinPayload.push_back(defRoom->getId());
        joinPayload.push_back(static_cast<uint8_t>(defRoom->getTcpPort() >> 8));
        joinPayload.push_back(
            static_cast<uint8_t>(defRoom->getTcpPort() & 0xFF));
        joinPayload.push_back(static_cast<uint8_t>(defRoom->getUdpPort() >> 8));
        joinPayload.push_back(
            static_cast<uint8_t>(defRoom->getUdpPort() & 0xFF));

        _session.sendTcp(playerId,
                         factory.createMessage(OpCode::JOIN, joinPayload));

        LOG_INFO("Lobby: Auto-joining player " + std::to_string(playerId) +
                 " to default room " + std::to_string(defRoom->getId()) +
                 " (TCP:" + std::to_string(defRoom->getTcpPort()) + ")");
    }
}

void LobbyHandler::handleRoomList(uint32_t playerId) {
    _roomManager.handleRoomListRequest(playerId);
}

void LobbyHandler::handleRoomCreate(uint32_t playerId,
                                    const std::vector<uint8_t> &data) {
    if (data.empty())
        return;
    std::string roomName(data.begin(), data.end());
    _roomManager.handleRoomCreate(playerId, roomName);
}

void LobbyHandler::handleRoomJoin(uint32_t playerId,
                                  const std::vector<uint8_t> &data) {
    if (data.empty())
        return;
    uint8_t roomId = data[0];
    _roomManager.handleRoomJoin(playerId, roomId);
}

void LobbyHandler::handleRoomLeave(uint32_t playerId) {
    _roomManager.handleRoomLeave(playerId);
}
