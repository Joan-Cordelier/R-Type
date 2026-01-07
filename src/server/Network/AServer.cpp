/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** AServer
*/

#include "AServer.hpp"
#include "../Logs/Logger.hpp"

AServer::AServer(ThreadedQueue<DecodedMessage> &queue) : _queue(queue) {}

int AServer::init(AServer::protocol protocol, int port) {
    int opt = 1;

    _port = port;
    _addrLen = sizeof(_addr);
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = INADDR_ANY;
    _addr.sin_port = htons(_port);
    _protocol = protocol;
    _serverFd = socket(AF_INET, protocol, 0);
    if (_serverFd < 0) {
        LOG_ERROR("Socket creation failed");
        return 84;
    }
    if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        LOG_ERROR("setsockopt SO_REUSEADDR failed");
        return 84;
    }
    if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) {
        LOG_ERROR("setsockopt SO_REUSEPORT failed");
        return 84;
    }
    if (bind(_serverFd, (struct sockaddr *)&_addr, sizeof(_addr)) < 0) {
        LOG_ERROR("Bind failed on port " + std::to_string(port));
        return 84;
    }

    // If port was 0 (dynamic), query the actual assigned port
    if (port == 0) {
        if (getsockname(_serverFd, (struct sockaddr *)&_addr, &_addrLen) < 0) {
            LOG_ERROR("getsockname failed");
            return 84;
        }
        _port = ntohs(_addr.sin_port);
    }

    if (protocol == TCP) {
        if (listen(_serverFd, 10) < 0) {
            LOG_ERROR("Listen failed");
            return 84;
        }
        LOG_INFO("TCP server listening on port " + std::to_string(_port));
    } else {
        LOG_INFO("UDP server listening on port " + std::to_string(_port));
    }

    return 0;
}

void AServer::reset() {
    _port = 0;
    _addrLen = 0;
    memset(&_addr, 0, sizeof(_addr));
    if (_serverFd >= 0) {
        close(_serverFd);
        _serverFd = -1;
    }
}

void AServer::handleDisconnections(const std::vector<int> &toDisconnect) {
    for (int fd : toDisconnect) {
        LOG_INFO("Client disconnected (fd: " + std::to_string(fd) + ")");
        close(fd);

        {
            std::lock_guard<std::mutex> lock(_clientsMutex);
            auto it = std::find(_clientFds.begin(), _clientFds.end(), fd);
            if (it != _clientFds.end())
                _clientFds.erase(it);
        }

        {
            std::lock_guard<std::mutex> lock(_fdsMutex);
            for (auto it2 = _fds.begin(); it2 != _fds.end(); ++it2) {
                if (it2->fd == fd) {
                    _fds.erase(it2);
                    break;
                }
            }
        }
    }
}

void AServer::stop() { _running.store(false); }
