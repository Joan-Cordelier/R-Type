#include "PrometheusExporter.hpp"

PrometheusExporter::PrometheusExporter(const std::string &bind)
    : _exposer(bind), _ready(false)
{
    registerMetrics();
}

void PrometheusExporter::registerMetrics()
{
    _registry = std::make_shared<prometheus::Registry>();
    _bytes_family = &prometheus::BuildCounter()
        .Name("rtype_net_bytes_total")
        .Help("Total bytes transferred via network")
        .Register(*_registry);

    _packet_type_family = &prometheus::BuildCounter()
        .Name("rtype_net_packet_types_total")
        .Help("Count of packets by type (MOVE, SHOOT, etc.)")
        .Register(*_registry);

    auto& loss_family = prometheus::BuildCounter()
        .Name("rtype_net_packet_loss_total")
        .Help("Total detected lost packets")
        .Register(*_registry);
    _packet_loss_counter = &loss_family.Add({});

    _loop_summary_family = &prometheus::BuildSummary()
        .Name("rtype_game_loop_duration_ms")
        .Help("Time spent in the game loop")
        .Register(*_registry);
    _loop_summary = &_loop_summary_family->Add({}, prometheus::Summary::Quantiles{{0.5, 0.05}, {0.9, 0.01}, {0.99, 0.001}});

    _entities_family = &prometheus::BuildGauge()
        .Name("rtype_ecs_entities_count")
        .Help("Total number of active entities in Registry")
        .Register(*_registry);
    _entities_gauge = &_entities_family->Add({});

    _queue_family = &prometheus::BuildGauge()
        .Name("rtype_message_queue_size")
        .Help("Number of messages waiting in the queue")
        .Register(*_registry);
    _queue_gauge = &_queue_family->Add({});

    _players_family = &prometheus::BuildGauge()
        .Name("rtype_connected_players")
        .Help("Number of connected players")
        .Register(*_registry);
    _players_gauge = &_players_family->Add({});

    _exposer.RegisterCollectable(_registry);
    _ready = true;
}

void PrometheusExporter::addBytes(const std::string& protocol, const std::string& direction, double bytes) {
    if (!_ready) return;
    
    std::string key = protocol + "_" + direction;
    
    std::lock_guard<std::mutex> lock(_bytes_mutex);
    if (_bytes_counters.find(key) == _bytes_counters.end()) {
        _bytes_counters[key] = &_bytes_family->Add({{"protocol", protocol}, {"direction", direction}});
    }
    _bytes_counters[key]->Increment(bytes);
}

void PrometheusExporter::recordPacketType(const std::string& packetName) {
    if (!_ready) return;

    std::lock_guard<std::mutex> lock(_type_mutex);
    if (_packet_type_counters.find(packetName) == _packet_type_counters.end()) {
        _packet_type_counters[packetName] = &_packet_type_family->Add({{"type", packetName}});
    }
    _packet_type_counters[packetName]->Increment();
}

void PrometheusExporter::incrementPacketLoss() {
    if (_ready) _packet_loss_counter->Increment();
}

void PrometheusExporter::updateGameLoopDuration(double ms) {
    if (_ready) _loop_summary->Observe(ms);
}

void PrometheusExporter::setEntityCount(int count) {
    if (_ready) _entities_gauge->Set(static_cast<double>(count));
}

void PrometheusExporter::setQueueSize(int size) {
    if (_ready) _queue_gauge->Set(static_cast<double>(size));
}

void PrometheusExporter::setPlayerCount(int n) {
    if (_ready) _players_gauge->Set(static_cast<double>(n));
}