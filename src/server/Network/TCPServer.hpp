/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** TCPServer
*/

#ifndef TCPSERVER_HPP_
#define TCPSERVER_HPP_

#include "AServer.hpp"
#include "../Data/LinearBuffer.hpp"
#include <map>

class TCPServer : public AServer {
    public:
        TCPServer(Queue& queue);
        ~TCPServer();
        int run();
    private:
        std::map<int, LinearBuffer> _buffers;
};

#endif /* !TCPSERVER_HPP_ */
