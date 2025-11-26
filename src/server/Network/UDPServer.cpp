/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** UDPServer
*/

#include "UDPServer.hpp"
#include "../Logs/Logger.hpp"
#include <arpa/inet.h>

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
        if (poll(_fds.data(), _fds.size(), 100) < 0) {
            LOG_ERROR("UDP poll failed");
            return 84;
        }
        for (size_t i = 0; i < _fds.size(); ++i) {
            if (_fds[i].revents & POLLIN) {
                char buffer[4096];
                struct sockaddr_in clientAddr;
                socklen_t len = sizeof(clientAddr);
                int n = recvfrom(_serverFd, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, &len);
                if (n > 0) {
                    bool isNewClient = false;
                    std::string clientIp = inet_ntoa(clientAddr.sin_addr);
                    int clientPort = ntohs(clientAddr.sin_port);
                    {
                        std::lock_guard<std::mutex> lock(_clientsMutex);
                        isNewClient = std::find_if(_clients.begin(), _clients.end(), [&clientAddr](const sockaddr_in& addr) {
                                return addr.sin_addr.s_addr == clientAddr.sin_addr.s_addr &&addr.sin_port == clientAddr.sin_port;
                            }) == _clients.end();
                        
                        if (isNewClient) {
                            _clients.push_back(clientAddr);
                            LOG_INFO("UDP client connected (" + clientIp + ":" + std::to_string(clientPort) + ")");
                        }
                    }
                    LOG_DEBUG("UDP received " + std::to_string(n) + " bytes from " + clientIp + ":" + std::to_string(clientPort));
                    std::vector<uint8_t> rawData(buffer, buffer + n);
                    DecodedMessage msg = factory.decode(rawData);
                    if (msg.opCode != PARSING_ERROR) {
                        LOG_INFO("UDP message received: OpCode=" + std::to_string(msg.opCode) + " Len=" + std::to_string(msg.len));
                        _queue.push(msg.priority, msg.data);
                    } else {
                        LOG_WARN("UDP parsing error from " + clientIp + ":" + std::to_string(clientPort));
                    }
                }
            }
        }
    }
    return 0;
}

int UDPServer::send(const MessageData& data, const sockaddr_in& clientAddr)
{
    std::string clientIp = inet_ntoa(clientAddr.sin_addr);
    int clientPort = ntohs(clientAddr.sin_port);
    ssize_t sent = sendto(_serverFd, data.data(), data.size(), 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
    
    if (sent < 0) {
        LOG_ERROR("UDP send failed to " + clientIp + ":" + std::to_string(clientPort));
        return -1;
    }
    LOG_DEBUG("UDP sent " + std::to_string(data.size()) + " bytes to " + clientIp + ":" + std::to_string(clientPort));
    return 0;
}

int UDPServer::send(const MessageData& data)
{
    std::lock_guard<std::mutex> lock(_clientsMutex);
    LOG_DEBUG("UDP broadcasting " + std::to_string(data.size()) + " bytes to " + std::to_string(_clients.size()) + " clients");
    for (const auto& clientAddr : _clients) {
        sendto(_serverFd, data.data(), data.size(), 0,
               (struct sockaddr*)&clientAddr, sizeof(clientAddr));
    }
    
    return 0;
}

