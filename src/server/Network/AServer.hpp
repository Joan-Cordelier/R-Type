/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** AServer
*/

#ifndef ASERVER_HPP_
#define ASERVER_HPP_

#include "../../common/Data/MessageFactory.hpp"
#include "../../common/Data/ThreadedQueue.hpp"
#include <arpa/inet.h>
#include <atomic>
#include <bits/stdc++.h>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

class AServer {
public:
    enum protocol { TCP = SOCK_STREAM, UDP = SOCK_DGRAM };
    AServer(ThreadedQueue<DecodedMessage> &queue);
    int init(AServer::protocol protocol, int port);
    int run();
    void stop();
    void reset();
    void handleDisconnections(const std::vector<int> &toDisconnect);

    // Get the actual bound port (useful when port=0 for dynamic assignment)
    uint16_t getPort() const {
        return static_cast<uint16_t>(ntohs(_addr.sin_port));
    }

protected:
    ThreadedQueue<DecodedMessage> &_queue;

    std::atomic<bool> _running{true};

    int _port = 0;
    int _serverFd = -1;
    protocol _protocol;
    struct sockaddr_in _addr;
    socklen_t _addrLen = 0;

    std::vector<struct pollfd> _fds;
    mutable std::mutex _fdsMutex;

    std::vector<int> _clientFds;
    std::vector<sockaddr_in> _clients;
    mutable std::mutex _clientsMutex;
};

#endif /* !ASERVER_HPP_ */
