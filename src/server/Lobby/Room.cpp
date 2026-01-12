/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Room
*/

#include "Room.hpp"
#include "../Logs/Logger.hpp"

Room::Room(uint32_t id, SessionManager &session, const std::string &configPath)
    : _id(id), _session(session), _inputQueue(std::make_shared<ThreadedQueue<DecodedMessage>>()) {
    // Initialize GameHandler but don't start the loop yet
    // GameHandler constructor expects running atomic ref
    _game = std::make_unique<GameHandler>(_session, _inputQueue, _running, configPath);
    LOG_INFO("Room " + std::to_string(id) + " created");
}

Room::~Room() {
    stop();
}

void Room::start() {
    if (_running)
        return;

    _running = true;
    _thread = std::thread(&GameHandler::run, _game.get());
    LOG_INFO("Room " + std::to_string(_id) + " started");
}

void Room::stop() {
    if (!_running)
        return;

    _running = false;
    if (_thread.joinable()) {
        _thread.join();
    }
    LOG_INFO("Room " + std::to_string(_id) + " stopped");
}

size_t Room::getPlayerCount() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _players.size();
}

bool Room::isFull() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _players.size() >= _maxPlayers;
}

void Room::addPlayer(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(_mutex);
    _players.push_back(playerId);

    // Notify session manager to update player's room ID if needed,
    // but usually LobbyManager handles that mapping.
    // However, GameHandler will receive CONNECT message and add player entity.
}

void Room::removePlayer(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(_mutex);
    for (auto it = _players.begin(); it != _players.end(); ++it) {
        if (*it == playerId) {
            _players.erase(it);

            // Notify GameHandler to remove entity
            DecodedMessage disMsg;
            disMsg.opCode = DISCONNECT;
            disMsg.playerId = playerId;
            disMsg.priority = Priority::CRITICAL;
            _inputQueue->push(Priority::CRITICAL, disMsg);
            break;
        }
    }
}

void Room::pushMessage(const DecodedMessage &msg) {
    _inputQueue->push(msg.priority, msg);
}

std::vector<uint32_t> Room::getPlayers() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _players;
}
