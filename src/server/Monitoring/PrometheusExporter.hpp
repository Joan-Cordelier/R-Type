#pragma once

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/family.h>
#include <prometheus/gauge.h>
#include <prometheus/histogram.h>
#include <prometheus/registry.h>
#include <prometheus/summary.h>
#include <string>

// Network statistics for admin console
struct NetworkStats {
    uint64_t tcpBytesSent = 0;
    uint64_t tcpBytesReceived = 0;
    uint64_t udpBytesSent = 0;
    uint64_t udpBytesReceived = 0;
    uint64_t totalMessagesSent = 0;
    uint64_t totalMessagesReceived = 0;
    std::map<std::string, uint64_t> messageCountsByType;
};

class PrometheusExporter {
public:
    PrometheusExporter(const std::string &bind);

    void addBytes(const std::string &protocol, const std::string &direction, double bytes);
    void recordPacketType(const std::string &packetName);
    void recordMessageSent(const std::string &opCodeName);
    void recordMessageReceived(const std::string &opCodeName);
    void incrementPacketLoss();
    void updateGameLoopDuration(double ms);
    void setEntityCount(int count);
    void setQueueSize(int size);
    void setPlayerCount(int n);

    NetworkStats getNetworkStats() const;

private:
    void registerMetrics();

    prometheus::Exposer _exposer;
    std::shared_ptr<prometheus::Registry> _registry;
    std::atomic<bool> _ready;
    prometheus::Family<prometheus::Counter> *_bytes_family = nullptr;
    std::map<std::string, prometheus::Counter *> _bytes_counters;
    mutable std::mutex _bytes_mutex;
    prometheus::Family<prometheus::Counter> *_packet_type_family = nullptr;
    std::map<std::string, prometheus::Counter *> _packet_type_counters;
    mutable std::mutex _type_mutex;
    prometheus::Counter *_packet_loss_counter = nullptr;
    prometheus::Family<prometheus::Summary> *_loop_summary_family = nullptr;
    prometheus::Summary *_loop_summary = nullptr;
    prometheus::Family<prometheus::Gauge> *_entities_family = nullptr;
    prometheus::Gauge *_entities_gauge = nullptr;
    prometheus::Family<prometheus::Gauge> *_queue_family = nullptr;
    prometheus::Gauge *_queue_gauge = nullptr;
    prometheus::Family<prometheus::Gauge> *_players_family = nullptr;
    prometheus::Gauge *_players_gauge = nullptr;

    // Internal counters for quick access (non-Prometheus)
    std::atomic<uint64_t> _tcpBytesSent{0};
    std::atomic<uint64_t> _tcpBytesReceived{0};
    std::atomic<uint64_t> _udpBytesSent{0};
    std::atomic<uint64_t> _udpBytesReceived{0};
    std::atomic<uint64_t> _totalMessagesSent{0};
    std::atomic<uint64_t> _totalMessagesReceived{0};
    std::map<std::string, std::atomic<uint64_t>> _messageCounts;
    mutable std::mutex _msgCountMutex;
};