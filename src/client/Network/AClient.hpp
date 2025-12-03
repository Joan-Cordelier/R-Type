/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** AClient
*/

#ifndef ACLIENT_HPP_
#define ACLIENT_HPP_

#include <iostream>
#include <vector>
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <cstring>
#include <mutex>
#include <atomic>
#include <errno.h>
#include "../../common/Data/MessageFactory.hpp"
#include "../../common/Data/ThreadedQueue.hpp"
#include "../../common/Data/OutgoingMessage.hpp"

class AClient {
    public:
        enum protocol {
            TCP = SOCK_STREAM,
            UDP = SOCK_DGRAM
        };

        AClient(ThreadedQueue<DecodedMessage>& queue);
        virtual ~AClient() = default;
        
        int init(AClient::protocol protocol, const std::string& serverIp, int port);
        virtual int connect() = 0;
        virtual int run() = 0;
        
        void stop();
        void reset();
        bool isConnected() const { return _connected.load(); }

    protected:
        ThreadedQueue<DecodedMessage>& _queue;
        ThreadedQueue<OutgoingMessage> _outgoingQueue;
        
        std::atomic<bool> _running{true};
        std::atomic<bool> _connected{false};
        
        int _port = 0;
        std::string _serverIp;
        protocol _protocol;
        struct sockaddr_in _serverAddr;
        socklen_t _addrLen = 0;
        
        int _socketFd = -1;
        mutable std::mutex _socketMutex;
};

#endif /* !ACLIENT_HPP_ */