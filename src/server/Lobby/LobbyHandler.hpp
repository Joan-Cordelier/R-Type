/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LobbyHandler - Handles lobby-specific protocol messages
*/

#ifndef LOBBYHANDLER_HPP_
#define LOBBYHANDLER_HPP_

#include "../../common/Data/MessageFactory.hpp"
#include "../Room/RoomManager.hpp"
#include "../Session/SessionManager.hpp"

class LobbyHandler {
public:
    LobbyHandler(SessionManager &session, RoomManager &roomManager);
    ~LobbyHandler() = default;

    // Process a message received by the lobby session
    void handleMessage(const DecodedMessage &msg);

private:
    SessionManager &_session;
    RoomManager &_roomManager;

    void handleConnect(uint32_t playerId);
    void handleRoomList(uint32_t playerId);
    void handleRoomCreate(uint32_t playerId, const std::vector<uint8_t> &data);
    void handleRoomJoin(uint32_t playerId, const std::vector<uint8_t> &data);
    void handleRoomLeave(uint32_t playerId);
};

#endif /* !LOBBYHANDLER_HPP_ */
