/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** AServer
*/

#include "AServer.hpp"

AServer::AServer(ThreadedQueue& queue) : _queue(queue)
{
}

int AServer::init(AServer::protocol protocol, int port)
{
    int opt = 1;

    _port = port;
    _addrLen = sizeof(_addr);
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = INADDR_ANY;
    _addr.sin_port = htons(_port);
    _protocol = protocol;
    _serverFd = socket(AF_INET, protocol, 0);
    if (_serverFd < 0) {
        perror("socket failed");
        return 84;
    }
    if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt SO_REUSEADDR failed");
        return 84;
    }
    if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt SO_REUSEPORT failed");
        return 84;
    }
    if (bind(_serverFd, (struct sockaddr*)&_addr, sizeof(_addr)) < 0) {
        perror("bind failed");
        return 84;
    }

    if (protocol == TCP) {
        if (listen(_serverFd, 10) < 0) {
            perror("listen failed");
            return 84;
        }
    }

    return 0;
}

void AServer::reset()
{
    _max = 0;
    _port = 0;
    _addrLen = 0;
    _error = 0;
    memset(&_addr, 0, sizeof(_addr));
    if (_serverFd >= 0) {
        close(_serverFd);
        _serverFd = -1;
    }
}

void AServer::handleDisconnections(const std::vector<int>& toDisconnect)
{
    for (int fd : toDisconnect) {
        close(fd);
        
        {
            std::lock_guard<std::mutex> lock(_clientsMutex);
            auto it = std::find(_clientFds.begin(), _clientFds.end(), fd);
            if (it != _clientFds.end()) _clientFds.erase(it);
        }

        for (auto it2 = _fds.begin(); it2 != _fds.end(); ++it2) {
            if (it2->fd == fd) {
                _fds.erase(it2);
                break;
            }
        }
    }
}

void AServer::stop()
{
    _running = false;
}
