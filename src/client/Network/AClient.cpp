/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** AClient
*/

#include "AClient.hpp"

AClient::AClient(ThreadedQueue<DecodedMessage> &queue) : _queue(queue) {}

int AClient::init(AClient::protocol protocol, const std::string &serverIp,
                  int port) {
    _running = true;
    _port = port;
    _serverIp = serverIp;
    _protocol = protocol;
    _addrLen = sizeof(_serverAddr);

    _socketFd = socket(AF_INET, protocol, 0);
    if (_socketFd < 0) {
        perror("socket failed");
        return 84;
    }

    memset(&_serverAddr, 0, sizeof(_serverAddr));
    _serverAddr.sin_family = AF_INET;
    _serverAddr.sin_port = htons(_port);

    if (inet_pton(AF_INET, _serverIp.c_str(), &_serverAddr.sin_addr) <= 0) {
        perror("invalid address");
        close(_socketFd);
        _socketFd = -1;
        return 84;
    }

    return 0;
}

void AClient::reset() {
    _running = false;
    _connected = false;
    _port = 0;
    _addrLen = 0;
    _serverIp.clear();
    memset(&_serverAddr, 0, sizeof(_serverAddr));

    if (_socketFd >= 0) {
        close(_socketFd);
        _socketFd = -1;
    }
}

void AClient::stop() {
    _running = false;
    _connected = false;
    if (_socketFd >= 0) {
        shutdown(_socketFd, SHUT_RDWR);
    }
}