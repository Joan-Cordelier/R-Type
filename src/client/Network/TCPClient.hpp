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
        TCPClient(ThreadedQueue<DecodedMessage>& queue);
        ~TCPClient();
        
        int connect() override;
        int run() override;
        int send(const MessageData& data, Priority priority = Priority::MEDIUM);
        
    private:
        LinearBuffer _buffer;
        
        void processOutgoingQueue();
        int sendData(const MessageData& data);
};

#endif /* !TCPCLIENT_HPP_ */