/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** TCPServer
*/

#include "TCPServer.hpp"

TCPServer::TCPServer(Queue& queue) : AServer(queue)
{
    init(AServer::protocol::TCP, 4789);
}

TCPServer::~TCPServer()
{
    reset();
}

int TCPServer::run()
{
    _fds.push_back({_serverFd, POLLIN, 0});

    while (_running) {
        if (poll(_fds.data(), _fds.size(), 100) < 0) {
            if (errno == EINTR)
                continue;
            perror("poll failed");
            return 84;
        }
        std::vector<int> toDisconnect;
        for (size_t i = 0; i < _fds.size(); ++i) {
            if (_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                 toDisconnect.push_back(_fds[i].fd);
                 continue;
            }
            if (_fds[i].revents & POLLIN) {
                if (_fds[i].fd == _serverFd) {
                    int newFd = accept(_serverFd, nullptr, nullptr);
                    if (newFd >= 0) {
                        _fds.push_back({newFd, POLLIN, 0});
                        _clientFds.push_back(newFd);
                        _buffers.emplace(newFd, LinearBuffer());
                    } else
                        perror("accept failed");
                } else {
                    char buffer[4096];
                    int n = recv(_fds[i].fd, buffer, sizeof(buffer), 0);
                    
                    if (n <= 0) {
                        if (n < 0)
                            perror("recv failed");
                        toDisconnect.push_back(_fds[i].fd);
                    } else {
                        if (!_buffers[_fds[i].fd].write(buffer, n)) {
                            toDisconnect.push_back(_fds[i].fd);
                            continue;
                        }
                        while (true) {
                            Packet packet = extractPacketFromLinearBuffer(_buffers[_fds[i].fd]);
                            if (packet.opCode == INCOMPLETE)
                                break;
                            if (packet.opCode == PARSING_ERROR) {
                                toDisconnect.push_back(_fds[i].fd);
                                break;
                            }
                            _queue.push(PriorityTable[packet.opCode], packet.data);
                        }
                    }
                }
            }
        }

        handleDisconnections(toDisconnect);        
        for (int fd : toDisconnect) {
            _buffers.erase(fd);
        }
    }
    return 0;
}
