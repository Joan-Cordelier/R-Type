/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** TCPServer
*/

#ifndef TCPSERVER_HPP_
#define TCPSERVER_HPP_

#include "../../common/Data/LinearBuffer.hpp"
#include "../../common/Data/MessageFactory.hpp"
#include "../../common/Data/OutgoingMessage.hpp"
#include "../../common/Data/ThreadedQueue.hpp"
#include "../Monitoring/PrometheusExporter.hpp"
#include "AServer.hpp"
#include <chrono>
#include <functional>
#include <map>

class TCPServer : public AServer {
public:
    using ConnectionCallback = std::function<void(int fd)>;
    using DisconnectionCallback = std::function<void(int fd)>;

    // Heartbeat configuration
    static constexpr int HEARTBEAT_TIMEOUT_SECONDS = 30; // Disconnect after 30s idle

    TCPServer(ThreadedQueue<DecodedMessage> &queue, PrometheusExporter &monitor);
    ~TCPServer();
    int run();

    void send(const MessageData &data, Priority priority = Priority::MEDIUM);
    void send(int fd, const MessageData &data, Priority priority = Priority::MEDIUM);
    void disconnectClient(int fd);

    void setOnConnect(ConnectionCallback callback) {
        _onConnect = callback;
    }
    void setOnDisconnect(DisconnectionCallback callback) {
        _onDisconnect = callback;
    }

    // Connection stats for admin console
    struct ClientStats {
        int fd;
        int secondsSinceActivity;
    };
    std::vector<ClientStats> getClientStats() const;

private:
    void processOutgoingQueue();
    void checkHeartbeats(std::vector<int> &toDisconnect);
    int sendToFd(int fd, const MessageData &data);
    int sendToAll(const MessageData &data);

    std::map<int, LinearBuffer> _buffers;
    std::map<int, std::chrono::steady_clock::time_point> _lastActivity;
    ThreadedQueue<OutgoingMessage> _outgoingQueue;
    mutable std::mutex _buffersMutex;

    ConnectionCallback _onConnect;
    DisconnectionCallback _onDisconnect;
    PrometheusExporter &_monitor;
};

#endif /* !TCPSERVER_HPP_ */
