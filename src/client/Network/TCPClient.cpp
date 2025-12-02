/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** TCPClient
*/

#include "TCPClient.hpp"

TCPClient::TCPClient(ThreadedQueue<DecodedMessage>& queue) : AClient(queue)
{
    init(AClient::protocol::TCP, "127.0.0.1", 4789);
}

TCPClient::~TCPClient()
{
    reset();
}

int TCPClient::connect()
{
    if (_socketFd < 0) {
        std::cerr << "Socket not initialized" << std::endl;
        return 84;
    }
    
    if (::connect(_socketFd, (struct sockaddr*)&_serverAddr, 
                  sizeof(_serverAddr)) < 0) {
        perror("connect failed");
        return 84;
    }
    
    _connected = true;
    return 0;
}

int TCPClient::run()
{
    if (!_connected) {
        std::cerr << "Client not connected" << std::endl;
        return 84;
    }
    
    MessageFactory& factory = MessageFactory::getInstance();
    struct pollfd pfd = {_socketFd, POLLIN, 0};
    
    while (_running) {
        int pollResult = poll(&pfd, 1, 100);
        
        if (pollResult < 0) {
            if (errno == EINTR)
                continue;
            perror("poll failed");
            return 84;
        }
        
        if (pollResult == 0)
            continue;
        
        if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
            std::cerr << "Connection closed or error detected" << std::endl;
            _connected = false;
            return 84;
        }
        
        if (pfd.revents & POLLIN) {
            char buffer[4096];
            int n = recv(_socketFd, buffer, sizeof(buffer), 0);
            
            if (n <= 0) {
                if (n < 0)
                    perror("recv failed");
                else
                    std::cerr << "Server closed connection" << std::endl;
                _connected = false;
                return 84;
            }

            if (!_buffer.write(buffer, n)) {
                std::cerr << "Buffer write failed (overflow)" << std::endl;
                _connected = false;
                return 84;
            }

            while (true) {
                DecodedMessage msg = factory.decodeFromBuffer(_buffer);
                
                if (msg.opCode == INCOMPLETE) {
                    break;
                } 
                if (msg.opCode == PARSING_ERROR) {
                    std::cerr << "Parsing error, disconnecting" << std::endl;
                    _connected = false;
                    return 84;
                }
                _queue.push(msg.priority, msg);
            }
        }
    }
    return 0;
}

int TCPClient::send(const MessageData& data)
{
    if (!_connected || data.empty()) {
        return -1;
    }

    std::lock_guard<std::mutex> lock(_socketMutex);
    ssize_t sent = ::send(_socketFd, data.data(), data.size(), MSG_NOSIGNAL);
    
    if (sent < 0) {
        perror("send failed");
        _connected = false;
        return -1;
    }
    return 0;
}