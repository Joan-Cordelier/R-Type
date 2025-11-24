/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** UDPServer
*/

#include "UDPServer.hpp"

UDPServer::UDPServer(Queue& queue) : AServer(queue)
{
    init(AServer::protocol::UDP, 4789);
}

UDPServer::~UDPServer()
{
    reset();
}

int UDPServer::run()
{
    _fds.push_back({_serverFd, POLLIN, 0});

    while (_running) {
        if (poll(_fds.data(), _fds.size(), 100) < 0)
            return 84;
        for (size_t i = 0; i < _fds.size(); ++i) {
            if (_fds[i].revents & POLLIN) {
                char buffer[4096];
                struct sockaddr_in clientAddr;
                socklen_t len = sizeof(clientAddr);
                int n = recvfrom(_serverFd, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, &len);
                if (n > 0) {
                    std::vector<uint8_t> rawData(buffer, buffer + n);
                    Packet packet = parsePacket(rawData);
                    if (packet.opCode != PARSING_ERROR) {
                        _queue.push(PriorityTable[packet.opCode], packet.data);
                    }
                }
            }
        }
    }
    return 0;
}

