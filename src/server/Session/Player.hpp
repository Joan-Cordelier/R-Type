/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Player
*/

#ifndef PLAYER_HPP_
#define PLAYER_HPP_

#include <cstdint>
#include <netinet/in.h>
#include <string>

struct Player {
    uint32_t id = 0;
    int tcpFd = -1;
    sockaddr_in udpAddr{};
    bool udpLinked = false;
    bool connected = false;
    uint8_t roomId = 0;
    
    uint32_t userId = 0;
    std::string username;
    bool isGuest = false;
    bool isAuthenticated = false;
    
    Player() = default;
    Player(uint32_t playerId, int fd) : id(playerId), tcpFd(fd) {}
};

#endif /* !PLAYER_HPP_ */
