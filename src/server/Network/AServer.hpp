/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** AServer
*/

#ifndef ASERVER_HPP_
#define ASERVER_HPP_

#include <iostream>
#include <bits/stdc++.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>
#include <sys/ioctl.h>
#include "../../common/Data/MessageFactory.hpp"
#include "../../common/Data/ThreadedQueue.hpp"
#include <mutex>

class AServer {
    public:
        enum protocol {
            TCP = SOCK_STREAM,
            UDP = SOCK_DGRAM
        };
        AServer(ThreadedQueue& queue);
        int init(AServer::protocol protocol, int port);
        int run();
        int send(const MessageData& data, const sockaddr_in& clientAddr);
        int send(const MessageData& data);
        void stop();
        void reset();
        void handleDisconnections(const std::vector<int>& toDisconnect);
    protected:
        ThreadedQueue& _queue;
        bool _running = true;
        int _max = 0;
        int _port = 0;
        int _serverFd = -1;
        protocol _protocol;
        std::vector<struct pollfd> _fds;
        std::vector<int> _clientFds;
        std::vector<sockaddr_in> _clients;
        struct sockaddr_in _addr;
        socklen_t _addrLen = 0;
        int _error = 0;
        mutable std::mutex _clientsMutex;
};

#endif /* !ASERVER_HPP_ */
