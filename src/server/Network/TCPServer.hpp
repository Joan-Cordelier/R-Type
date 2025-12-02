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

class TCPServer : public AServer {
    public:
        TCPServer(ThreadedQueue<DecodedMessage>& queue);
        ~TCPServer();
        int run();
        
        // Queue message for sending (thread-safe)
        void send(const MessageData& data, Priority priority = Priority::MEDIUM);
        void send(int fd, const MessageData& data, Priority priority = Priority::MEDIUM);
        
    private:
        void processOutgoingQueue();
        int sendToFd(int fd, const MessageData& data);
        int sendToAll(const MessageData& data);
        
        std::map<int, LinearBuffer> _buffers;
        ThreadedQueue<OutgoingMessage> _outgoingQueue;
};

#endif /* !TCPSERVER_HPP_ */
