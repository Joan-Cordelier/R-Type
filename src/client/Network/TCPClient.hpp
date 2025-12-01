/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** TCPClient
*/

#ifndef TCPCLIENT_HPP_
#define TCPCLIENT_HPP_

#include "AClient.hpp"
#include "../../common/Data/LinearBuffer.hpp"
#include "../../common/Data/MessageFactory.hpp"

class TCPClient : public AClient {
    public:
        TCPClient(ThreadedQueue& queue);
        ~TCPClient();
        
        int connect(std::string ip_adress) override;
        int run() override;
        int send(const MessageData& data) override;
        
    private:
        LinearBuffer _buffer;
};

#endif /* !TCPCLIENT_HPP_ */