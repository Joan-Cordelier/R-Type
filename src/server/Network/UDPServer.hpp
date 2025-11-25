/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** UDPServer
*/

#ifndef UDPSERVER_HPP_
#define UDPSERVER_HPP_

#include "AServer.hpp"

class UDPServer : public AServer {
    public:
        UDPServer(Queue& queue);
        ~UDPServer();
        int run();
};

#endif /* !UDPSERVER_HPP_ */
