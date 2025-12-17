/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** NetworkManager
*/

#ifndef NETWORKMANAGER_HPP_
#define NETWORKMANAGER_HPP_

#include "TCPClient.hpp"
#include "UDPClient.hpp"
#include "../../common/Data/ThreadedQueue.hpp"
#include <thread>
#include <atomic>
#include <string>

class NetworkManager {
    public:
        NetworkManager();
        ~NetworkManager();
        
        int connect(const std::string& serverIp, int tcpPort, int udpPort);
        void disconnect();
        bool isConnected() const;
        bool isRunning() const { return _running.load(); }
        
        void start();
        void stop();
        
        int sendTcp(const MessageData& data, Priority priority = Priority::MEDIUM);
        int sendUdp(const MessageData& data);
        
        int sendTcp(const PreparedMessage& msg);
        int sendUdp(const PreparedMessage& msg);
        
        std::optional<DecodedMessage> popMessage(Priority priority);
        bool hasMessages() const;
        
        TCPClient& getTcpClient() { return _tcpClient; }
        UDPClient& getUdpClient() { return _udpClient; }
        ThreadedQueue<DecodedMessage>& getQueue() { return _queue; }

    private:
        ThreadedQueue<DecodedMessage> _queue;
        
        TCPClient _tcpClient;
        UDPClient _udpClient;
        
        std::thread _tcpThread;
        std::thread _udpThread;
        
        std::atomic<bool> _running{false};
};

#endif /* !NETWORKMANAGER_HPP_ */
