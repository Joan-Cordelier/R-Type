#pragma once

#include <memory>
#include <string>
#include <atomic>
#include <map>
#include <mutex>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>
#include <prometheus/family.h> 
#include <prometheus/counter.h> 
#include <prometheus/gauge.h>   
#include <prometheus/summary.h>  
#include <prometheus/histogram.h> 

class PrometheusExporter {
public:
    PrometheusExporter(const std::string &bind);

    void addBytes(const std::string& protocol, const std::string& direction, double bytes);
    void recordPacketType(const std::string& packetName);
    void incrementPacketLoss();
    void updateGameLoopDuration(double ms);
    void setEntityCount(int count);
    void setQueueSize(int size);
    void setPlayerCount(int n);

private:
    void registerMetrics();

    prometheus::Exposer _exposer;
    std::shared_ptr<prometheus::Registry> _registry;
    std::atomic<bool> _ready;
    prometheus::Family<prometheus::Counter>* _bytes_family = nullptr;
    std::map<std::string, prometheus::Counter*> _bytes_counters; 
    std::mutex _bytes_mutex;
    prometheus::Family<prometheus::Counter>* _packet_type_family = nullptr;
    std::map<std::string, prometheus::Counter*> _packet_type_counters;
    std::mutex _type_mutex;
    prometheus::Counter* _packet_loss_counter = nullptr;
    prometheus::Family<prometheus::Summary>* _loop_summary_family = nullptr;
    prometheus::Summary* _loop_summary = nullptr;
    prometheus::Family<prometheus::Gauge>* _entities_family = nullptr;
    prometheus::Gauge* _entities_gauge = nullptr;
    prometheus::Family<prometheus::Gauge>* _queue_family = nullptr;
    prometheus::Gauge* _queue_gauge = nullptr;
    prometheus::Family<prometheus::Gauge>* _players_family = nullptr;
    prometheus::Gauge* _players_gauge = nullptr;
};