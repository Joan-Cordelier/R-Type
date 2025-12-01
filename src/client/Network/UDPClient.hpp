/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** UDPClient
*/

#ifndef UDPCLIENT_HPP_
#define UDPCLIENT_HPP_

#include "AClient.hpp"
#include "../../common/Data/MessageFactory.hpp"

class UDPClient : public AClient {
    public:
        UDPClient(ThreadedQueue<DecodedMessage>& queue);
        ~UDPClient();
        
        int connect(std::string ip_adress) override;
        int run() override;
        int send(const MessageData& data) override;
};

#endif /* !UDPCLIENT_HPP_ */