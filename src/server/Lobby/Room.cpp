/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Room
*/

#include "Room.hpp"
#include "../Logs/Logger.hpp"
#include "../Database/UserDatabase.hpp"

Room::Room(uint32_t id, SessionManager &session, const std::string &configPath, 
           PrometheusExporter& monitor, const RoomConfig& config)
    : _id(id)
    , _session(session)
    , _monitor(monitor)
    , _config(config)
    , _inputQueue(std::make_shared<ThreadedQueue<DecodedMessage>>())
    , _emptyTimestamp(std::chrono::steady_clock::now())
    , _wasEmpty(true)
{
    // Initialize GameHandler but don't start the loop yet
    // GameHandler constructor expects running atomic ref
    _game = std::make_unique<GameHandler>(_session, _inputQueue, _running, configPath, monitor, _config.difficulty);

    // Set up callback for when players die
    _game->setOnPlayerDeath([this](uint32_t playerId) {
        onPlayerDeath(playerId);
    });

    LOG_INFO("Room " + std::to_string(id) + " created (MaxPlayers: " + 
             std::to_string(_config.maxPlayers) + ", Mode: " + _config.getGameModeStr() + 
             ", Difficulty: " + _config.getDifficultyStr() + ")");
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

    // Clear the death callback to avoid dangling references
    _game->setOnPlayerDeath(nullptr);

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
    return _players.size() >= _config.maxPlayers;
}

void Room::addPlayer(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(_mutex);
    _players.push_back(playerId);
    
    // Track all participants for scoring at game end
    bool alreadyParticipant = false;
    for (auto pid : _allParticipants) {
        if (pid == playerId) {
            alreadyParticipant = true;
            break;
        }
    }
    if (!alreadyParticipant) {
        _allParticipants.push_back(playerId);
    }
    
    _wasEmpty = false;  // Room is no longer empty

    // Notify session manager to update player's room ID if needed,
    // but usually LobbyManager handles that mapping.
    // However, GameHandler will receive CONNECT message and add player entity.
}

void Room::removePlayer(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(_mutex);
    for (auto it = _players.begin(); it != _players.end(); ++it) {
        if (*it == playerId) {
            _players.erase(it);

            // Track when room becomes empty
            if (_players.empty() && !_wasEmpty) {
                _emptyTimestamp = std::chrono::steady_clock::now();
                _wasEmpty = true;
            }

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
    std::string opName = MessageFactory::getInstance().getOpCodeName(msg.opCode);
    _monitor.recordPacketType("Game_" + opName);
    _inputQueue->push(msg.priority, msg);
}

std::vector<uint32_t> Room::getPlayers() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _players;
}

bool Room::hasBeenEmptyFor(std::chrono::seconds duration) const {
    std::lock_guard<std::mutex> lock(_mutex);
    if (!_players.empty()) {
        return false;
    }
    auto now = std::chrono::steady_clock::now();
    return (now - _emptyTimestamp) >= duration;
}

void Room::onPlayerDeath(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(_mutex);
    for (auto it = _players.begin(); it != _players.end(); ++it) {
        if (*it == playerId) {
            _players.erase(it);
            LOG_INFO("Player " + std::to_string(playerId) + " removed from room " +
                     std::to_string(_id) + " after death");

            // Track when room becomes empty (game over)
            if (_players.empty() && !_wasEmpty) {
                _emptyTimestamp = std::chrono::steady_clock::now();
                _wasEmpty = true;

                // Game over - save scores for all players who participated
                if (_game) {
                    int finalScore = _game->getScore();
                    LOG_INFO("Game over in room " + std::to_string(_id) + 
                             " with final score: " + std::to_string(finalScore));

                    // Update score for ALL players who participated in this game
                    auto& userDb = UserDatabase::getInstance();
                    for (uint32_t participantId : _allParticipants) {
                        auto* participant = _session.getPlayer(participantId);
                        if (participant && participant->userId > 0) {
                            userDb.updateScore(participant->userId, finalScore);
                            LOG_INFO("Updated score for participant " + participant->username + 
                                     " (userId: " + std::to_string(participant->userId) + ")");
                        }
                    }
                }
            }
            break;
        }
    }
}
