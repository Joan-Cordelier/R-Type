/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RoomManager - Manages multiple game rooms
*/

#ifndef ROOMMANAGER_HPP_
#define ROOMMANAGER_HPP_

#include "../Session/SessionManager.hpp"
#include "Room.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

/**
 * @brief Information about a room for listing purposes.
 */
struct RoomInfo {
    uint8_t id;
    std::string name;
    uint8_t playerCount;
    bool started;
    uint16_t tcpPort;
    uint16_t udpPort;
};

/**
 * @brief Manages multiple game rooms.
 *
 * The RoomManager is responsible for:
 * - Creating and destroying rooms
 * - Providing room listings for clients
 * - Coordinating player room assignments with lobby
 *
 * Each Room owns its own network (TCP+UDP on dynamic ports).
 */
class RoomManager {
public:
    RoomManager(SessionManager &lobbySession);
    ~RoomManager();

    // Room lifecycle
    Room *createRoom(const std::string &name);
    Room *getRoom(uint8_t roomId);
    bool deleteRoom(uint8_t roomId);

    // Room queries
    std::vector<RoomInfo> listRooms() const;
    size_t getRoomCount() const;

    // Process room-related protocol messages (from lobby)
    void handleRoomListRequest(uint32_t playerId);
    void handleRoomCreate(uint32_t playerId, const std::string &roomName);
    void handleRoomJoin(uint32_t playerId, uint8_t roomId);
    void handleRoomLeave(uint32_t playerId);

    // Get default room (for backward compatibility)
    Room *getDefaultRoom();

private:
    SessionManager &_lobbySession; // Lobby session for sending responses

    std::unordered_map<uint8_t, std::unique_ptr<Room>> _rooms;
    mutable std::shared_mutex _roomsMutex;

    uint8_t _nextRoomId = 1;
    uint8_t _defaultRoomId = 0; // Will be set when first room is created
};

#endif /* !ROOMMANAGER_HPP_ */
