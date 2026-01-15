/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** UDPServer
*/

#include "UDPServer.hpp"
#include "../Logs/Logger.hpp"
#include <arpa/inet.h>

UDPServer::UDPServer(ThreadedQueue<DecodedMessage>& queue, PrometheusExporter& monitor)
    : AServer(queue), _monitor(monitor)
{
    init(AServer::protocol::UDP, 4790);
}

UDPServer::~UDPServer()
{
    reset();
}

int UDPServer::run()
{
    {
        std::lock_guard<std::mutex> lock(_fdsMutex);
        _fds.push_back({_serverFd, POLLIN, 0});
    }
    MessageFactory& factory = MessageFactory::getInstance();

    while (_running) {
        std::vector<struct pollfd> fdsSnapshot;
        {
            std::lock_guard<std::mutex> lock(_fdsMutex);
            fdsSnapshot = _fds;
        }
        
        if (poll(fdsSnapshot.data(), fdsSnapshot.size(), 100) < 0) {
            LOG_ERROR("UDP poll failed");
            return 84;
        }
        
        for (size_t i = 0; i < fdsSnapshot.size(); ++i) {
            if (fdsSnapshot[i].revents & POLLIN) {
                char buffer[4096];
                struct sockaddr_in clientAddr;
                socklen_t len = sizeof(clientAddr);
                int n = recvfrom(_serverFd, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, &len);
                if (n > 0) {
                    _monitor.addBytes("udp", "rx", static_cast<double>(n));
                    bool isNewClient = false;
                    std::string clientIp = inet_ntoa(clientAddr.sin_addr);
                    int clientPort = ntohs(clientAddr.sin_port);
                    {
                        std::lock_guard<std::mutex> lock(_clientsMutex);
                        isNewClient = std::find_if(_clients.begin(), _clients.end(), [&clientAddr](const sockaddr_in& addr) {
                                return addr.sin_addr.s_addr == clientAddr.sin_addr.s_addr && addr.sin_port == clientAddr.sin_port;
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
                        msg.udpAddr = clientAddr;
                        LOG_INFO("UDP message received: OpCode=" + std::to_string(msg.opCode) + " Len=" + std::to_string(msg.len));
                        _queue.push(msg.priority, msg);
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
    if (data.empty())
        return -1;
        
    std::string clientIp = inet_ntoa(clientAddr.sin_addr);
    int clientPort = ntohs(clientAddr.sin_port);
    ssize_t sent = sendto(_serverFd, data.data(), data.size(), 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
    
    if (sent > 0) {
    _monitor.addBytes("udp", "tx", static_cast<double>(sent));
    }
    if (sent < 0) {
        LOG_ERROR("UDP send failed to " + clientIp + ":" + std::to_string(clientPort));
        return -1;
    }
    
    if (sent != static_cast<ssize_t>(data.size())) {
        LOG_ERROR("UDP partial send to " + clientIp + ":" + std::to_string(clientPort) + ": sent " + std::to_string(sent) + "/" + std::to_string(data.size()) + " bytes");
        return -1;
    }
    
    LOG_DEBUG("UDP sent " + std::to_string(data.size()) + " bytes to " + clientIp + ":" + std::to_string(clientPort));
    return 0;
}

int UDPServer::send(const MessageData& data)
{
    if (data.empty())
        return -1;
        
    std::lock_guard<std::mutex> lock(_clientsMutex);
    LOG_DEBUG("UDP broadcasting " + std::to_string(data.size()) + " bytes to " + std::to_string(_clients.size()) + " clients");
    
    int result = 0;
    for (const auto& clientAddr : _clients) {
        ssize_t sent = sendto(_serverFd, data.data(), data.size(), 0,
                             (struct sockaddr*)&clientAddr, sizeof(clientAddr));
        if (sent > 0) {
        _monitor.addBytes("udp", "tx", static_cast<double>(sent));
        }
        if (sent < 0 || sent != static_cast<ssize_t>(data.size())) {
            std::string clientIp = inet_ntoa(clientAddr.sin_addr);
            int clientPort = ntohs(clientAddr.sin_port);
            LOG_ERROR("UDP broadcast send failed to " + clientIp + ":" + std::to_string(clientPort));
            result = -1;
        }
    }
    
    return result;
}

