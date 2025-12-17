/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** NetworkManager
*/

#include "NetworkManager.hpp"
#include <iostream>

NetworkManager::NetworkManager() 
    : _tcpClient(_queue), _udpClient(_queue)
{
}

NetworkManager::~NetworkManager()
{
    disconnect();
}

int NetworkManager::connect(const std::string& serverIp, int tcpPort, int udpPort)
{
    if (_tcpClient.init(AClient::protocol::TCP, serverIp, tcpPort) != 0) {
        std::cerr << "Failed to initialize TCP client" << std::endl;
        return -1;
    }
    
    if (_udpClient.init(AClient::protocol::UDP, serverIp, udpPort) != 0) {
        std::cerr << "Failed to initialize UDP client" << std::endl;
        return -1;
    }
    
    if (_tcpClient.connect() != 0) {
        std::cerr << "Failed to connect TCP client" << std::endl;
        return -1;
    }
    
    if (_udpClient.connect() != 0) {
        std::cerr << "Failed to connect UDP client" << std::endl;
        return -1;
    }
    
    std::cout << "Connected to server " << serverIp << " (TCP:" << tcpPort << ", UDP:" << udpPort << ")" << std::endl;
    return 0;
}

void NetworkManager::disconnect()
{
    stop();
    
    _tcpClient.reset();
    _udpClient.reset();
    
    std::cout << "Disconnected from server" << std::endl;
}

bool NetworkManager::isConnected() const
{
    return _tcpClient.isConnected() && _udpClient.isConnected();
}

void NetworkManager::start()
{
    if (_running.load())
        return;
    
    if (!isConnected()) {
        std::cerr << "Cannot start NetworkManager: not connected" << std::endl;
        return;
    }
    
    _running.store(true);
    std::cout << "Starting NetworkManager..." << std::endl;
    
    _tcpThread = std::thread(&TCPClient::run, &_tcpClient);
    _udpThread = std::thread(&UDPClient::run, &_udpClient);
    
    std::cout << "NetworkManager started - TCP and UDP clients running" << std::endl;
}

void NetworkManager::stop()
{
    if (!_running.load())
        return;
    
    std::cout << "Stopping NetworkManager..." << std::endl;
    _running.store(false);
    
    _tcpClient.stop();
    _udpClient.stop();
    
    if (_tcpThread.joinable())
        _tcpThread.join();
    if (_udpThread.joinable())
        _udpThread.join();
    
    std::cout << "NetworkManager stopped" << std::endl;
}

int NetworkManager::sendTcp(const MessageData& data, Priority priority)
{
    if (!_tcpClient.isConnected()) {
        std::cerr << "Cannot send TCP: not connected" << std::endl;
        return -1;
    }
    return _tcpClient.send(data, priority);
}

int NetworkManager::sendTcp(const PreparedMessage& msg)
{
    return sendTcp(msg.data, msg.priority);
}

int NetworkManager::sendUdp(const MessageData& data)
{
    if (!_udpClient.isConnected()) {
        std::cerr << "Cannot send UDP: not connected" << std::endl;
        return -1;
    }
    return _udpClient.send(data);
}

int NetworkManager::sendUdp(const PreparedMessage& msg)
{
    return sendUdp(msg.data);
}

std::optional<DecodedMessage> NetworkManager::popMessage(Priority priority)
{
    return _queue.pop(priority);
}

bool NetworkManager::hasMessages() const
{
    return !_queue.isEmpty();
}
