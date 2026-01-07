/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Connection - Network connection state (separate from game state)
*/

#ifndef CONNECTION_HPP_
#define CONNECTION_HPP_

#include <cstdint>
#include <netinet/in.h>

/**
 * @brief Network connection state for a player.
 *
 * This struct contains ONLY network-related fields.
 * Game state (room membership, entity data) is managed by RoomManager/Room.
 */
struct Connection {
    uint32_t playerId = 0;
    int tcpFd = -1;
    sockaddr_in udpAddr{};
    bool udpLinked = false;
    bool connected = false;

    Connection() = default;
    Connection(uint32_t id, int fd)
        : playerId(id), tcpFd(fd), connected(true) {}
};

#endif /* !CONNECTION_HPP_ */
