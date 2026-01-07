/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RoomManager implementation
*/

#include "RoomManager.hpp"
#include "../Logs/Logger.hpp"

RoomManager::RoomManager(SessionManager &lobbySession)
    : _lobbySession(lobbySession) {
    LOG_INFO("RoomManager initialized");
}

RoomManager::~RoomManager() {
    // Stop all rooms
    std::unique_lock<std::shared_mutex> lock(_roomsMutex);
    for (auto &[id, room] : _rooms) {
        room->stop();
    }
    _rooms.clear();
    LOG_INFO("RoomManager destroyed");
}

Room *RoomManager::createRoom(const std::string &name) {
    std::unique_lock<std::shared_mutex> lock(_roomsMutex);

    uint8_t roomId = _nextRoomId++;
    auto room = std::make_unique<Room>(roomId, name);

    // Start the room (it will create its own network on dynamic ports)
    room->start();

    Room *ptr = room.get();
    _rooms[roomId] = std::move(room);

    // Set as default room if this is the first room
    if (_defaultRoomId == 0) {
        _defaultRoomId = roomId;
        LOG_INFO("Default room set to " + std::to_string(roomId));
    }

    LOG_INFO("Created room " + std::to_string(roomId) + ": " + name +
             " (TCP:" + std::to_string(ptr->getTcpPort()) +
             ", UDP:" + std::to_string(ptr->getUdpPort()) + ")");
    return ptr;
}

Room *RoomManager::getRoom(uint8_t roomId) {
    std::shared_lock<std::shared_mutex> lock(_roomsMutex);
    auto it = _rooms.find(roomId);
    return (it != _rooms.end()) ? it->second.get() : nullptr;
}

bool RoomManager::deleteRoom(uint8_t roomId) {
    std::unique_lock<std::shared_mutex> lock(_roomsMutex);

    auto it = _rooms.find(roomId);
    if (it == _rooms.end()) {
        return false;
    }

    it->second->stop();
    _rooms.erase(it);

    LOG_INFO("Deleted room " + std::to_string(roomId));
    return true;
}

std::vector<RoomInfo> RoomManager::listRooms() const {
    std::shared_lock<std::shared_mutex> lock(_roomsMutex);

    std::vector<RoomInfo> result;
    result.reserve(_rooms.size());

    for (const auto &[id, room] : _rooms) {
        RoomInfo info;
        info.id = room->getId();
        info.name = room->getName();
        info.playerCount = static_cast<uint8_t>(room->getPlayerCount());
        info.started = room->isStarted();
        info.tcpPort = room->getTcpPort();
        info.udpPort = room->getUdpPort();
        result.push_back(info);
    }

    return result;
}

size_t RoomManager::getRoomCount() const {
    std::shared_lock<std::shared_mutex> lock(_roomsMutex);
    return _rooms.size();
}

void RoomManager::handleRoomListRequest(uint32_t playerId) {
    auto rooms = listRooms();

    // Build ROOM_LIST message: [count (1B)] [roomId (1B), playerCount (1B),
    // tcpPort (2B), udpPort (2B), nameLen (1B), name...] ...
    MessageData payload;
    payload.push_back(static_cast<uint8_t>(rooms.size()));

    for (const auto &room : rooms) {
        payload.push_back(room.id);
        payload.push_back(room.playerCount);
        payload.push_back(static_cast<uint8_t>(room.tcpPort >> 8));
        payload.push_back(static_cast<uint8_t>(room.tcpPort & 0xFF));
        payload.push_back(static_cast<uint8_t>(room.udpPort >> 8));
        payload.push_back(static_cast<uint8_t>(room.udpPort & 0xFF));
        payload.push_back(static_cast<uint8_t>(room.name.size()));
        for (char c : room.name) {
            payload.push_back(static_cast<uint8_t>(c));
        }
    }

    auto &factory = MessageFactory::getInstance();
    PreparedMessage msg = factory.createMessage(OpCode::ROOM_LIST, payload);
    _lobbySession.sendTcp(playerId, msg);

    LOG_DEBUG("Sent room list to player " + std::to_string(playerId));
}

void RoomManager::handleRoomCreate(uint32_t playerId,
                                   const std::string &roomName) {
    Room *room = createRoom(roomName);
    if (room) {
        // Send JOIN_ACK with room ports
        MessageData payload;
        payload.push_back(room->getId());
        payload.push_back(static_cast<uint8_t>(room->getTcpPort() >> 8));
        payload.push_back(static_cast<uint8_t>(room->getTcpPort() & 0xFF));
        payload.push_back(static_cast<uint8_t>(room->getUdpPort() >> 8));
        payload.push_back(static_cast<uint8_t>(room->getUdpPort() & 0xFF));

        auto &factory = MessageFactory::getInstance();
        PreparedMessage msg = factory.createMessage(OpCode::JOIN, payload);
        _lobbySession.sendTcp(playerId, msg);

        LOG_INFO("Player " + std::to_string(playerId) +
                 " created and will join room " +
                 std::to_string(room->getId()) +
                 " on TCP:" + std::to_string(room->getTcpPort()) +
                 " UDP:" + std::to_string(room->getUdpPort()));
    }
}

void RoomManager::handleRoomJoin(uint32_t playerId, uint8_t roomId) {
    Room *room = getRoom(roomId);
    if (!room) {
        LOG_WARN("Player " + std::to_string(playerId) +
                 " tried to join non-existent room " + std::to_string(roomId));
        return;
    }

    // Send JOIN_ACK with room ports
    MessageData payload;
    payload.push_back(roomId);
    payload.push_back(static_cast<uint8_t>(room->getTcpPort() >> 8));
    payload.push_back(static_cast<uint8_t>(room->getTcpPort() & 0xFF));
    payload.push_back(static_cast<uint8_t>(room->getUdpPort() >> 8));
    payload.push_back(static_cast<uint8_t>(room->getUdpPort() & 0xFF));

    auto &factory = MessageFactory::getInstance();
    PreparedMessage msg = factory.createMessage(OpCode::JOIN, payload);
    _lobbySession.sendTcp(playerId, msg);

    LOG_INFO("Player " + std::to_string(playerId) + " will join room " +
             std::to_string(roomId) +
             " on TCP:" + std::to_string(room->getTcpPort()) +
             " UDP:" + std::to_string(room->getUdpPort()));
}

void RoomManager::handleRoomLeave(uint32_t playerId) {
    // In new architecture, leaving is handled by room's SessionManager
    // disconnect
    LOG_INFO("Player " + std::to_string(playerId) + " requested to leave room");
}

Room *RoomManager::getDefaultRoom() { return getRoom(_defaultRoomId); }
