/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LobbyManager
*/

#ifndef LOBBYMANAGER_HPP_
#define LOBBYMANAGER_HPP_

#include "../Session/SessionManager.hpp"
#include "Room.hpp"
#include <atomic>
#include <map>
#include <memory>
#include <thread>

class LobbyManager {
public:
    LobbyManager(const std::string &configPath = "yaml/main_loop.yaml");
    ~LobbyManager();

    void run();
    void stop();

private:
    SessionManager _session;
    std::map<uint32_t, std::shared_ptr<Room>> _rooms;
    std::atomic<bool> _running{true};
    std::string _configPath;

    uint32_t _nextRoomId = 1;

    void processMessages();
    void dispatchMessage(DecodedMessage &msg);

    void handleCreateRoom(const DecodedMessage &msg);
    void handleJoinRoom(const DecodedMessage &msg);
    void handleListRooms(const DecodedMessage &msg);
    void handleConnect(const DecodedMessage &msg);
    void handleLink(const DecodedMessage &msg);

    // Handling disconnections
    void onPlayerDisconnect(const Player &player);
};

#endif /* !LOBBYMANAGER_HPP_ */
