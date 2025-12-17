/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** UDPClient
*/

#include "UDPClient.hpp"

UDPClient::UDPClient(ThreadedQueue<DecodedMessage>& queue) : AClient(queue)
{
}

UDPClient::~UDPClient()
{
    reset();
}

int UDPClient::connect()
{
    if (_socketFd < 0) {
        std::cerr << "Socket not initialized" << std::endl;
        return 84;
    }
    
    _connected.store(true);
    return 0;
}

int UDPClient::run()
{
    if (!_connected.load()) {
        std::cerr << "Client not connected" << std::endl;
        return 84;
    }
    
    MessageFactory& factory = MessageFactory::getInstance();
    struct pollfd pfd = {_socketFd, POLLIN, 0};
    
    while (_running.load()) {
        int pollResult = poll(&pfd, 1, 100);
        
        if (pollResult < 0) {
            if (errno == EINTR)
                continue;
            perror("poll failed");
            return 84;
        }
        
        if (pollResult == 0)
            continue;
            
        if (pfd.revents & POLLIN) {
            char buffer[4096];
            struct sockaddr_in fromAddr;
            socklen_t fromLen = sizeof(fromAddr);
            
            int n = recvfrom(_socketFd, buffer, sizeof(buffer), 0,
                           (struct sockaddr*)&fromAddr, &fromLen);
            
            if (n > 0) {
                std::vector<uint8_t> rawData(buffer, buffer + n);
                DecodedMessage msg = factory.decode(rawData);
                
                if (msg.opCode != PARSING_ERROR) {
                    _queue.push(msg.priority, msg);
                } else {
                    std::cerr << "Parsing error received from server" << std::endl;
                }
            } else if (n < 0) {
                perror("recvfrom failed");
            }
        }
        if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
            std::cerr << "Socket error detected" << std::endl;
            _connected.store(false);
            return 84;
        }
    }
    return 0;
}

int UDPClient::send(const MessageData& data)
{
    if (data.empty() || !_connected.load()) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(_socketMutex);
    ssize_t sent = sendto(_socketFd, data.data(), data.size(), 0,
                         (struct sockaddr*)&_serverAddr, sizeof(_serverAddr));
    
    if (sent < 0) {
        perror("UDP send failed");
        return -1;
    }
    
    if (sent != static_cast<ssize_t>(data.size())) {
        std::cerr << "UDP partial send: sent " << sent << "/" << data.size() << " bytes" << std::endl;
        return -1;
    }
    
    return 0;
}