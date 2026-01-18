/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Room
*/

#include "Room.hpp"
#include "../Database/UserDatabase.hpp"
#include "../Logs/Logger.hpp"

Room::Room(uint32_t id, SessionManager &session, const std::string &configPath,
           PrometheusExporter &monitor, const RoomConfig &config)
    : _id(id), _session(session), _monitor(monitor), _config(config),
      _inputQueue(std::make_shared<ThreadedQueue<DecodedMessage>>()),
      _emptyTimestamp(std::chrono::steady_clock::now()), _wasEmpty(true) {
    // Initialize GameHandler but don't start the loop yet
    // GameHandler constructor expects running atomic ref
    _game = std::make_unique<GameHandler>(_session, _inputQueue, _running, configPath, monitor,
                                          _config.difficulty, _config.gameMode);

    // Set up callback for when players die
    _game->setOnPlayerDeath([this](uint32_t playerId) { onPlayerDeath(playerId); });

    // Set up callback for when game is won (victory)
    _game->setOnGameVictory([this](uint32_t finalScore) { onGameVictory(finalScore); });

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

    _wasEmpty = false; // Room is no longer empty

    // Notify session manager to update player's room ID if needed,
    // but usually LobbyManager handles that mapping.
    // However, GameHandler will receive CONNECT message and add player entity.
}

void Room::removePlayer(uint32_t playerId) {
    // Lookup player info and call the overloaded version
    auto *player = _session.getPlayer(playerId);
    if (player) {
        removePlayer(playerId, player->userId, player->username);
    } else {
        // Player already gone, just remove from list without banning
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto it = _players.begin(); it != _players.end(); ++it) {
            if (*it == playerId) {
                _players.erase(it);

                if (_players.empty() && !_wasEmpty) {
                    _emptyTimestamp = std::chrono::steady_clock::now();
                    _wasEmpty = true;
                }

                DecodedMessage disMsg;
                disMsg.opCode = DISCONNECT;
                disMsg.playerId = playerId;
                disMsg.priority = Priority::CRITICAL;
                _inputQueue->push(Priority::CRITICAL, disMsg);
                break;
            }
        }
    }
}

void Room::removePlayer(uint32_t playerId, uint32_t userId, const std::string &username) {
    std::lock_guard<std::mutex> lock(_mutex);
    for (auto it = _players.begin(); it != _players.end(); ++it) {
        if (*it == playerId) {
            _players.erase(it);

            // Ban player from rejoining this room (no come back policy)
            // Use userId for registered users, username for guests
            std::string banId = userId > 0 ? "user:" + std::to_string(userId) : "guest:" + username;
            _bannedUsers.insert(banId);
            LOG_INFO("User " + banId + " banned from room " + std::to_string(_id) +
                     " (disconnected/left)");

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

            // Ban player from rejoining this room (no come back policy)
            // Use userId for registered users, username for guests
            auto *player = _session.getPlayer(playerId);
            if (player) {
                std::string banId = player->userId > 0 ? "user:" + std::to_string(player->userId)
                                                       : "guest:" + player->username;
                _bannedUsers.insert(banId);
                LOG_INFO("User " + banId + " banned from room " + std::to_string(_id) + " (died)");
            }
            LOG_INFO("Player " + std::to_string(playerId) + " removed from room " +
                     std::to_string(_id) + " after death");

            // Track when room becomes empty (game over)
            if (_players.empty() && !_wasEmpty) {
                _emptyTimestamp = std::chrono::steady_clock::now();
                _wasEmpty = true;

                // Game over - save scores for all players who participated
                if (_game) {
                    int finalScore = _game->getScore();
                    LOG_INFO("=== GAME OVER === Room " + std::to_string(_id) +
                             " - Final score: " + std::to_string(finalScore) +
                             " - Participants: " + std::to_string(_allParticipants.size()) +
                             " - Mode: " + std::to_string(static_cast<int>(_config.gameMode)));

                    // Update score for ALL players who participated in this game
                    auto &userDb = UserDatabase::getInstance();
                    for (uint32_t participantId : _allParticipants) {
                        auto *participant = _session.getPlayer(participantId);
                        if (participant && participant->userId > 0) {
                            // Use different score saving based on game mode
                            if (_config.gameMode == GameMode::ENDLESS) {
                                userDb.updateEndlessScore(participant->userId,
                                                          static_cast<uint8_t>(_config.difficulty),
                                                          static_cast<uint32_t>(finalScore));
                                LOG_INFO("Updated endless score for participant " +
                                         participant->username + " (userId: " +
                                         std::to_string(participant->userId) + ", difficulty: " +
                                         std::to_string(static_cast<int>(_config.difficulty)) +
                                         ", score: " + std::to_string(finalScore) + ")");
                            } else {
                                userDb.updateScore(participant->userId, finalScore);
                                LOG_INFO("Updated score for participant " + participant->username +
                                         " (userId: " + std::to_string(participant->userId) +
                                         ", score: " + std::to_string(finalScore) + ")");
                            }
                        } else if (participant) {
                            LOG_INFO("Skipping guest player: " + participant->username);
                        }
                    }
                } else {
                    LOG_WARN("Game pointer is null at game over!");
                }
            }
            break;
        }
    }
}

void Room::banPlayer(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto *player = _session.getPlayer(playerId);
    if (player) {
        std::string banId = player->userId > 0 ? "user:" + std::to_string(player->userId)
                                               : "guest:" + player->username;
        _bannedUsers.insert(banId);
        LOG_INFO("User " + banId + " manually banned from room " + std::to_string(_id));
    }
}

bool Room::isPlayerBanned(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto *player = _session.getPlayer(playerId);
    if (!player)
        return false;

    std::string banId =
        player->userId > 0 ? "user:" + std::to_string(player->userId) : "guest:" + player->username;
    return _bannedUsers.find(banId) != _bannedUsers.end();
}

void Room::onGameVictory(uint32_t finalScore) {
    LOG_INFO("=== VICTORY === Room " + std::to_string(_id) +
             " - Final score: " + std::to_string(finalScore));

    // Send VICTORY message to all players
    auto &factory = MessageFactory::getInstance();
    MessageData payload = factory.encodeMessageVictory(finalScore);
    PreparedMessage msg = factory.createMessage(OpCode::VICTORY, payload);

    std::lock_guard<std::mutex> lock(_mutex);
    for (uint32_t playerId : _players) {
        _session.sendTcp(playerId, msg);
    }

    // Save scores for all participants
    auto &userDb = UserDatabase::getInstance();
    for (uint32_t participantId : _allParticipants) {
        auto *participant = _session.getPlayer(participantId);
        if (participant && participant->userId > 0) {
            // Only save endless scores for Endless mode (registered users only)
            if (_config.gameMode == GameMode::ENDLESS) {
                userDb.updateEndlessScore(participant->userId,
                                          static_cast<uint8_t>(_config.difficulty),
                                          static_cast<uint32_t>(finalScore));
                LOG_INFO("Updated endless score for " + participant->username +
                         " (userId: " + std::to_string(participant->userId) +
                         ", difficulty: " + std::to_string(static_cast<int>(_config.difficulty)) +
                         ", score: " + std::to_string(finalScore) + ")");
            } else {
                // Other modes use regular high score
                userDb.updateScore(participant->userId, static_cast<uint32_t>(finalScore));
                LOG_INFO("Updated victory score for " + participant->username +
                         " (userId: " + std::to_string(participant->userId) +
                         ", score: " + std::to_string(finalScore) + ")");
            }
        }
    }
}
