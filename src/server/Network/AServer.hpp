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
#include <atomic>

class AServer {
    public:
        enum protocol {
            TCP = SOCK_STREAM,
            UDP = SOCK_DGRAM
        };
        AServer(ThreadedQueue<DecodedMessage>& queue);
        int init(AServer::protocol protocol, int port);
        int run();
        void stop();
        void reset();
        void handleDisconnections(const std::vector<int>& toDisconnect);
    protected:
        // Queue for incoming messages
        ThreadedQueue<DecodedMessage>& _queue;
        
        // Server state
        std::atomic<bool> _running{true};
        
        // Server configuration
        int _port = 0;
        int _serverFd = -1;
        protocol _protocol;
        struct sockaddr_in _addr;
        socklen_t _addrLen = 0;
        
        // File descriptors management
        std::vector<struct pollfd> _fds;
        mutable std::mutex _fdsMutex;
        
        // Client management
        std::vector<int> _clientFds;
        std::vector<sockaddr_in> _clients;
        mutable std::mutex _clientsMutex;
};

#endif /* !ASERVER_HPP_ */
