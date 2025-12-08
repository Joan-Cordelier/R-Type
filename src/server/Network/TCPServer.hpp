/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** TCPServer
*/

#ifndef TCPSERVER_HPP_
#define TCPSERVER_HPP_

#include "AServer.hpp"
#include "../../common/Data/LinearBuffer.hpp"
#include "../../common/Data/MessageFactory.hpp"
#include "../../common/Data/ThreadedQueue.hpp"
#include "../../common/Data/OutgoingMessage.hpp"
#include <map>
#include <functional>

class TCPServer : public AServer {
    public:
        using ConnectionCallback = std::function<void(int fd)>;
        using DisconnectionCallback = std::function<void(int fd)>;
        
        TCPServer(ThreadedQueue<DecodedMessage>& queue);
        ~TCPServer();
        int run();
        
        void send(const MessageData& data, Priority priority = Priority::MEDIUM);
        void send(int fd, const MessageData& data, Priority priority = Priority::MEDIUM);
        
        void setOnConnect(ConnectionCallback callback) { _onConnect = callback; }
        void setOnDisconnect(DisconnectionCallback callback) { _onDisconnect = callback; }
        
    private:
        void processOutgoingQueue();
        int sendToFd(int fd, const MessageData& data);
        int sendToAll(const MessageData& data);
        
        std::map<int, LinearBuffer> _buffers;
        ThreadedQueue<OutgoingMessage> _outgoingQueue;
        mutable std::mutex _buffersMutex;
        
        ConnectionCallback _onConnect;
        DisconnectionCallback _onDisconnect;
};

#endif /* !TCPSERVER_HPP_ */
