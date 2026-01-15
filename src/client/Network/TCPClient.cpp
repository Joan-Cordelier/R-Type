/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** TCPClient
*/

#include "TCPClient.hpp"

TCPClient::TCPClient(ThreadedQueue<DecodedMessage>& queue) : AClient(queue)
{
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
    
    _connected.store(true);
    return 0;
}

int TCPClient::run()
{
    if (!_connected.load()) {
        std::cerr << "Client not connected" << std::endl;
        return 84;
    }
    
    MessageFactory& factory = MessageFactory::getInstance();
    struct pollfd pfd = {_socketFd, POLLIN, 0};
    
    int loopCount = 0;
    while (_running.load()) {
        loopCount++;
        if (loopCount % 50 == 1) {  // Log every 5 seconds (50 * 100ms)
            std::cout << "[TCPClient] Loop iteration " << loopCount << " running=" << _running.load() << std::endl;
        }
        int pollResult = poll(&pfd, 1, 100);
        
        if (pollResult < 0) {
            if (errno == EINTR)
                continue;
            perror("poll failed");
            return 84;
        }
        
        processOutgoingQueue();
        
        if (pollResult == 0)
            continue;
        
        if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
            std::cerr << "Connection closed or error detected" << std::endl;
            _connected.store(false);
            return 84;
        }
        
        if (pfd.revents & POLLIN) {
            char buffer[4096];
            int n = recv(_socketFd, buffer, sizeof(buffer), 0);
            
            std::cout << "[TCPClient] Received " << n << " bytes from server" << std::endl;
            
            if (n <= 0) {
                if (n < 0)
                    perror("recv failed");
                else
                    std::cerr << "Server closed connection" << std::endl;
                _connected.store(false);
                return 84;
            }

            if (!_buffer.write(buffer, n)) {
                std::cerr << "Buffer write failed (overflow)" << std::endl;
                _connected.store(false);
                return 84;
            }

            std::cout << "[TCPClient] Buffer size after write: " << _buffer.size() << std::endl;
            std::cout << "[TCPClient] First bytes: ";
            for (size_t i = 0; i < std::min(_buffer.size(), (size_t)10); ++i) {
                std::cout << std::hex << (int)_buffer.peek(i) << " ";
            }
            std::cout << std::dec << std::endl;

            while (true) {
                std::cout << "[TCPClient] Attempting decode, buffer size: " << _buffer.size() << std::endl;
                DecodedMessage msg = factory.decodeFromBuffer(_buffer);
                
                std::cout << "[TCPClient] Decode result: OpCode=" << static_cast<int>(msg.opCode) 
                          << " (INCOMPLETE=" << static_cast<int>(INCOMPLETE) << ")" << std::endl;
                
                if (msg.opCode == INCOMPLETE) {
                    break;
                } 
                if (msg.opCode == PARSING_ERROR) {
                    std::cerr << "Parsing error, disconnecting" << std::endl;
                    _connected.store(false);
                    return 84;
                }
                std::cout << "[TCPClient] Received message OpCode: " << static_cast<int>(msg.opCode)
                          << " Size: " << msg.len << " Priority: " << static_cast<int>(msg.priority)
                          << std::endl;
                _queue.push(msg.priority, msg);
            }
        }
    }
    return 0;
}

int TCPClient::send(const MessageData& data, Priority priority)
{
    if (data.empty())
        return -1;
    _outgoingQueue.push(priority, {-1, data});
    return 0;
}

void TCPClient::processOutgoingQueue()
{
    for (Priority p : {Priority::CRITICAL, Priority::HIGH, Priority::MEDIUM, Priority::LOW}) {
        while (true) {
            auto msgOpt = _outgoingQueue.pop(p);
            if (!msgOpt.has_value())
                break;
            
            OutgoingMessage& msg = msgOpt.value();
            sendData(msg.data);
        }
    }
}

int TCPClient::sendData(const MessageData& data)
{
    if (data.empty() || !_connected.load())
        return -1;
    
    std::lock_guard<std::mutex> lock(_socketMutex);
    
    ssize_t sent = ::send(_socketFd, data.data(), data.size(), MSG_NOSIGNAL);
    if (sent < 0) {
        perror("TCP send failed");
        _connected.store(false);
        return -1;
    }
    
    if (sent != static_cast<ssize_t>(data.size())) {
        std::cerr << "TCP partial send: sent " << sent << "/" << data.size() << " bytes" << std::endl;
        _connected.store(false);
        return -1;
    }
    
    return 0;
}