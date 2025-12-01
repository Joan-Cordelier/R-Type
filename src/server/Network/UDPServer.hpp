/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** UDPServer
*/

#ifndef UDPSERVER_HPP_
#define UDPSERVER_HPP_

#include "AServer.hpp"
#include "../../common/Data/MessageFactory.hpp"

class UDPServer : public AServer {
    public:
        UDPServer(ThreadedQueue<DecodedMessage>& queue);
        ~UDPServer();
        int run();
        int send(const MessageData& data, const sockaddr_in& clientAddr);
        int send(const MessageData& data);
};

#endif /* !UDPSERVER_HPP_ */
