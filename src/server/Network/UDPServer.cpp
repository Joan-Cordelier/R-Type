/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** UDPServer
*/

#include "UDPServer.hpp"

UDPServer::UDPServer(ThreadedQueue& queue) : AServer(queue)
{
    init(AServer::protocol::UDP, 4789);
}

UDPServer::~UDPServer()
{
    reset();
}

int UDPServer::run()
{
    _fds.push_back({_serverFd, POLLIN, 0});
    MessageFactory& factory = MessageFactory::getInstance();

    while (_running) {
        if (poll(_fds.data(), _fds.size(), 100) < 0)
            return 84;
        for (size_t i = 0; i < _fds.size(); ++i) {
            if (_fds[i].revents & POLLIN) {
                char buffer[4096];
                struct sockaddr_in clientAddr;
                socklen_t len = sizeof(clientAddr);
                int n = recvfrom(_serverFd, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, &len);
                if (n > 0) {
                    bool isNewClient = false;
                    {
                        std::lock_guard<std::mutex> lock(_clientsMutex);
                        isNewClient = std::find_if(_clients.begin(), _clients.end(), [&clientAddr](const sockaddr_in& addr) {
                                return addr.sin_addr.s_addr == clientAddr.sin_addr.s_addr &&addr.sin_port == clientAddr.sin_port;
                            }) == _clients.end();
                        
                        if (isNewClient)
                            _clients.push_back(clientAddr);
                    }
                    std::vector<uint8_t> rawData(buffer, buffer + n);
                    DecodedMessage msg = factory.decode(rawData);
                    if (msg.opCode != PARSING_ERROR)
                        _queue.push(msg.priority, msg.data);
                }
            }
        }
    }
    return 0;
}

int UDPServer::send(const MessageData& data, const sockaddr_in& clientAddr)
{
    ssize_t sent = sendto(_serverFd, data.data(), data.size(), 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
    
    return (sent < 0) ? -1 : 0;
}

int UDPServer::send(const MessageData& data)
{
    std::lock_guard<std::mutex> lock(_clientsMutex);
    for (const auto& clientAddr : _clients) {
        sendto(_serverFd, data.data(), data.size(), 0,
               (struct sockaddr*)&clientAddr, sizeof(clientAddr));
    }
    
    return 0;
}

