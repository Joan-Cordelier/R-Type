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
#include <errno.h>
#include "../../common/Data/MessageFactory.hpp"
#include "../../common/Data/ThreadedQueue.hpp"

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
        virtual int send(const MessageData& data) = 0;
        
        void stop();
        void reset();
        bool isConnected() const { return _connected; }

    protected:
        ThreadedQueue<DecodedMessage>& _queue;
        bool _running = true;
        bool _connected = false;
        int _socketFd = -1;
        int _port = 0;
        std::string _serverIp;
        protocol _protocol;
        struct sockaddr_in _serverAddr;
        socklen_t _addrLen = 0;
        mutable std::mutex _socketMutex;
};

#endif /* !ACLIENT_HPP_ */