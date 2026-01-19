/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** UDPServer
*/

#ifndef UDPSERVER_HPP_
#define UDPSERVER_HPP_

#include "../../common/Data/MessageFactory.hpp"
#include "../Monitoring/PrometheusExporter.hpp"
#include "AServer.hpp"
#include <map>
#include <mutex>

// UDP sequence header (prepended to all UDP packets)
struct UDPSequenceHeader {
    uint16_t sequence;    // Packet sequence number
    uint16_t ackSequence; // Last received sequence (for RTT estimation)
};

class UDPServer : public AServer {
public:
    UDPServer(ThreadedQueue<DecodedMessage> &queue, PrometheusExporter &monitor);
    ~UDPServer();
    int run();
    int send(const MessageData &data, const sockaddr_in &clientAddr);
    int send(const MessageData &data);

    // Per-client UDP statistics
    struct UDPClientStats {
        uint16_t sendSequence = 0;     // Next sequence to send
        uint16_t lastRecvSequence = 0; // Last received sequence
        uint32_t packetsSent = 0;
        uint32_t packetsReceived = 0;
        uint32_t packetsOutOfOrder = 0; // Detected out-of-order/lost
    };

    std::vector<std::pair<std::string, UDPClientStats>> getUdpStats() const;

private:
    std::string addrToKey(const sockaddr_in &addr) const;
    uint16_t getNextSendSequence(const sockaddr_in &addr);
    void updateRecvSequence(const sockaddr_in &addr, uint16_t seq);

    PrometheusExporter &_monitor;
    std::map<std::string, UDPClientStats> _clientStats;
    mutable std::mutex _statsMutex;
};

#endif /* !UDPSERVER_HPP_ */
