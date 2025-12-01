/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** TCPServer
*/

#ifndef TCPSERVER_HPP_
#define TCPSERVER_HPP_

#include "AServer.hpp"
#include "../../common/Data/LinearBuffer.hpp"
#include "../../common/Data/MessageFactory.hpp"
#include <map>

class TCPServer : public AServer {
    public:
        TCPServer(ThreadedQueue<DecodedMessage>& queue);
        ~TCPServer();
        int run();
        int send(const MessageData& data, const sockaddr_in& clientAddr);
        int send(const MessageData& data);
    private:
        std::map<int, LinearBuffer> _buffers;
};

#endif /* !TCPSERVER_HPP_ */
