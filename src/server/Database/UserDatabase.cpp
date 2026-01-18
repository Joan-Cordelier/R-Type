/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** UserDatabase - JSON-based user storage implementation
*/

#include "UserDatabase.hpp"
#include "../Logs/Logger.hpp"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <openssl/sha.h>
#include <sstream>

namespace {
std::string sha256(const std::string &input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char *>(input.c_str()), input.size(), hash);

    std::ostringstream result;
    result << std::hex << std::setfill('0');
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        result << std::setw(2) << static_cast<int>(hash[i]);
    }
    return result.str();
}
} // namespace

UserDatabase &UserDatabase::getInstance() {
    static UserDatabase instance;
    return instance;
}

UserDatabase::UserDatabase() {
    load();
}

std::string UserDatabase::hashPassword(const std::string &password) {
    // Add salt for better security
    std::string salted = "Stupid_rtype" + password + "goated_glados";
    return sha256(salted);
}

uint32_t UserDatabase::generateUserId() {
    return _nextUserId++;
}

std::string UserDatabase::generateGuestUsername() {
    std::string username = "Guest_" + std::to_string(_nextGuestNumber++);
    return username;
}

bool UserDatabase::isValidUsername(const std::string &username) {
    if (username.length() < 3 || username.length() > 20)
        return false;
    for (char c : username) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_')
            return false;
    }
    if (username.rfind("Guest_", 0) == 0)
        return false;
    return true;
}

uint32_t UserDatabase::registerUser(const std::string &username, const std::string &password) {
    std::lock_guard<std::mutex> lock(_mutex);

    if (!isValidUsername(username)) {
        LOG_WARN("Registration failed: invalid username '" + username + "'");
        return 0;
    }
    if (_usernameIndex.find(username) != _usernameIndex.end()) {
        LOG_WARN("Registration failed: username '" + username + "' already exists");
        return 0;
    }
    if (password.length() < 4) {
        LOG_WARN("Registration failed: password too short");
        return 0;
    }

    User user;
    user.id = generateUserId();
    user.username = username;
    user.passwordHash = hashPassword(password);
    user.createdAt = std::chrono::duration_cast<std::chrono::seconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count();
    user.lastLogin = user.createdAt;
    user.isGuest = false;

    _users[user.id] = user;
    _usernameIndex[username] = user.id;

    LOG_DEBUG("User registered: " + username + " (ID: " + std::to_string(user.id) + ")");

    save();
    return user.id;
}

std::optional<User> UserDatabase::login(const std::string &username, const std::string &password) {
    std::lock_guard<std::mutex> lock(_mutex);

    auto it = _usernameIndex.find(username);
    if (it == _usernameIndex.end()) {
        LOG_DEBUG("Login failed: user '" + username + "' not found");
        return std::nullopt;
    }

    User &user = _users[it->second];

    if (user.passwordHash != hashPassword(password)) {
        LOG_DEBUG("Login failed: wrong password for '" + username + "'");
        return std::nullopt;
    }

    user.lastLogin = std::chrono::duration_cast<std::chrono::seconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count();

    LOG_DEBUG("User logged in: " + username + " (ID: " + std::to_string(user.id) + ")");

    save();
    return user;
}

User UserDatabase::createGuest() {
    std::lock_guard<std::mutex> lock(_mutex);

    User guest;
    guest.id = generateUserId();
    guest.username = generateGuestUsername();
    guest.passwordHash = "";
    guest.createdAt = std::chrono::duration_cast<std::chrono::seconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count();
    guest.lastLogin = guest.createdAt;
    guest.isGuest = true;

    _users[guest.id] = guest;
    _usernameIndex[guest.username] = guest.id;

    LOG_DEBUG("Guest created: " + guest.username + " (ID: " + std::to_string(guest.id) + ")");

    return guest;
}

std::optional<User> UserDatabase::getUserById(uint32_t userId) {
    std::lock_guard<std::mutex> lock(_mutex);

    auto it = _users.find(userId);
    if (it != _users.end())
        return it->second;
    return std::nullopt;
}

std::optional<User> UserDatabase::getUserByUsername(const std::string &username) {
    std::lock_guard<std::mutex> lock(_mutex);

    auto it = _usernameIndex.find(username);
    if (it != _usernameIndex.end())
        return _users[it->second];
    return std::nullopt;
}

bool UserDatabase::usernameExists(const std::string &username) {
    // Note: caller should hold lock if needed
    return _usernameIndex.find(username) != _usernameIndex.end();
}

bool UserDatabase::isGuest(uint32_t userId) {
    std::lock_guard<std::mutex> lock(_mutex);

    auto it = _users.find(userId);
    if (it != _users.end())
        return it->second.isGuest;
    return false;
}

void UserDatabase::updateLastLogin(uint32_t userId) {
    std::lock_guard<std::mutex> lock(_mutex);

    auto it = _users.find(userId);
    if (it != _users.end()) {
        it->second.lastLogin = std::chrono::duration_cast<std::chrono::seconds>(
                                   std::chrono::system_clock::now().time_since_epoch())
                                   .count();
        save();
    }
}

void UserDatabase::save() {
    // Ensure directory exists
    std::filesystem::create_directories("data");

    std::ofstream file(_filePath);
    if (!file.is_open()) {
        LOG_ERROR("Failed to save database: cannot open " + _filePath);
        return;
    }

    file << "{\n";
    file << "  \"nextUserId\": " << _nextUserId << ",\n";
    file << "  \"users\": [\n";

    bool first = true;
    for (const auto &[id, user] : _users) {
        if (user.isGuest)
            continue;

        if (!first)
            file << ",\n";
        first = false;

        file << "    {\n";
        file << "      \"id\": " << user.id << ",\n";
        file << "      \"username\": \"" << user.username << "\",\n";
        file << "      \"passwordHash\": \"" << user.passwordHash << "\",\n";
        file << "      \"createdAt\": " << user.createdAt << ",\n";
        file << "      \"lastLogin\": " << user.lastLogin << ",\n";
        file << "      \"highScore\": " << user.highScore << ",\n";
        file << "      \"totalGamesPlayed\": " << user.totalGamesPlayed << ",\n";
        // Save endless high scores per difficulty
        file << "      \"endlessHighScores\": {";
        bool firstScore = true;
        for (const auto &[diff, score] : user.endlessHighScores) {
            if (!firstScore)
                file << ", ";
            firstScore = false;
            file << "\"" << static_cast<int>(diff) << "\": " << score;
        }
        file << "}\n";
        file << "    }";
    }

    file << "\n  ]\n";
    file << "}\n";

    file.close();
    LOG_DEBUG("Database saved to " + _filePath);
}

void UserDatabase::load() {
    std::ifstream file(_filePath);
    if (!file.is_open()) {
        LOG_DEBUG("No existing database found, starting fresh");
        return;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // Parse nextUserId
    size_t pos = content.find("\"nextUserId\":");
    if (pos != std::string::npos) {
        pos += 13;
        while (pos < content.size() && (content[pos] == ' ' || content[pos] == '\t'))
            pos++;
        _nextUserId = std::stoul(content.substr(pos));
    }

    // Parse users array
    pos = content.find("\"users\":");
    if (pos == std::string::npos)
        return;

    size_t arrayStart = content.find('[', pos);
    size_t arrayEnd = content.rfind(']');
    if (arrayStart == std::string::npos || arrayEnd == std::string::npos)
        return;

    std::string usersStr = content.substr(arrayStart, arrayEnd - arrayStart + 1);

    // Find each user object
    size_t objStart = 0;
    while ((objStart = usersStr.find('{', objStart)) != std::string::npos) {
        size_t objEnd = usersStr.find('}', objStart);
        if (objEnd == std::string::npos)
            break;

        std::string userStr = usersStr.substr(objStart, objEnd - objStart + 1);

        User user;
        user.isGuest = false;

        // Parse id
        pos = userStr.find("\"id\":");
        if (pos != std::string::npos) {
            pos += 5;
            user.id = std::stoul(userStr.substr(pos));
        }

        // Parse username
        pos = userStr.find("\"username\":");
        if (pos != std::string::npos) {
            pos = userStr.find('"', pos + 11);
            size_t end = userStr.find('"', pos + 1);
            user.username = userStr.substr(pos + 1, end - pos - 1);
        }

        // Parse passwordHash
        pos = userStr.find("\"passwordHash\":");
        if (pos != std::string::npos) {
            pos = userStr.find('"', pos + 15);
            size_t end = userStr.find('"', pos + 1);
            user.passwordHash = userStr.substr(pos + 1, end - pos - 1);
        }

        // Parse createdAt
        pos = userStr.find("\"createdAt\":");
        if (pos != std::string::npos) {
            pos += 12;
            user.createdAt = std::stoull(userStr.substr(pos));
        }

        // Parse lastLogin
        pos = userStr.find("\"lastLogin\":");
        if (pos != std::string::npos) {
            pos += 12;
            user.lastLogin = std::stoull(userStr.substr(pos));
        }

        // Parse highScore
        pos = userStr.find("\"highScore\":");
        if (pos != std::string::npos) {
            pos += 12;
            user.highScore = std::stoul(userStr.substr(pos));
        }

        // Parse totalGamesPlayed
        pos = userStr.find("\"totalGamesPlayed\":");
        if (pos != std::string::npos) {
            pos += 19;
            user.totalGamesPlayed = std::stoul(userStr.substr(pos));
        }

        // Parse endlessHighScores
        pos = userStr.find("\"endlessHighScores\":");
        if (pos != std::string::npos) {
            size_t braceStart = userStr.find('{', pos);
            size_t braceEnd = userStr.find('}', braceStart);
            if (braceStart != std::string::npos && braceEnd != std::string::npos) {
                std::string scoresStr = userStr.substr(braceStart + 1, braceEnd - braceStart - 1);
                // Parse key-value pairs like "0": 1000, "1": 2000
                size_t scorePos = 0;
                while ((scorePos = scoresStr.find('"', scorePos)) != std::string::npos) {
                    size_t keyEnd = scoresStr.find('"', scorePos + 1);
                    if (keyEnd == std::string::npos)
                        break;
                    uint8_t diffKey = static_cast<uint8_t>(
                        std::stoi(scoresStr.substr(scorePos + 1, keyEnd - scorePos - 1)));
                    size_t colonPos = scoresStr.find(':', keyEnd);
                    if (colonPos == std::string::npos)
                        break;
                    size_t valueStart = colonPos + 1;
                    while (valueStart < scoresStr.size() && scoresStr[valueStart] == ' ')
                        valueStart++;
                    size_t valueEnd = scoresStr.find_first_of(",}", valueStart);
                    if (valueEnd == std::string::npos)
                        valueEnd = scoresStr.size();
                    uint32_t scoreVal =
                        std::stoul(scoresStr.substr(valueStart, valueEnd - valueStart));
                    user.endlessHighScores[diffKey] = scoreVal;
                    scorePos = valueEnd;
                }
            }
        }

        if (user.id > 0 && !user.username.empty()) {
            _users[user.id] = user;
            _usernameIndex[user.username] = user.id;
            LOG_DEBUG("Loaded user: " + user.username + " (ID: " + std::to_string(user.id) + ")");
        }

        objStart = objEnd + 1;
    }

    LOG_INFO("Loaded " + std::to_string(_users.size()) + " users from " + _filePath);
}

void UserDatabase::updateScore(uint32_t userId, uint32_t score) {
    std::lock_guard<std::mutex> lock(_mutex);

    auto it = _users.find(userId);
    if (it != _users.end()) {
        it->second.totalGamesPlayed++;
        if (score > it->second.highScore) {
            it->second.highScore = score;
            LOG_DEBUG("New high score for " + it->second.username + ": " + std::to_string(score));
        }
        save();
    }
}

std::vector<std::pair<std::string, uint32_t>> UserDatabase::getTopScores(size_t limit) {
    std::lock_guard<std::mutex> lock(_mutex);

    // Collect all non-guest users with their scores
    std::vector<std::pair<std::string, uint32_t>> scores;
    for (const auto &[id, user] : _users) {
        if (!user.isGuest && user.highScore > 0) {
            scores.emplace_back(user.username, user.highScore);
        }
    }

    // Sort by score descending
    std::sort(scores.begin(), scores.end(),
              [](const auto &a, const auto &b) { return a.second > b.second; });

    // Limit results
    if (scores.size() > limit) {
        scores.resize(limit);
    }

    return scores;
}

void UserDatabase::updateEndlessScore(uint32_t userId, uint8_t difficulty, uint32_t score) {
    std::lock_guard<std::mutex> lock(_mutex);

    auto it = _users.find(userId);
    if (it != _users.end()) {
        // Only save for registered users (not guests)
        if (it->second.isGuest) {
            LOG_DEBUG("Skipping endless score for guest user");
            return;
        }

        it->second.totalGamesPlayed++;
        uint32_t currentBest = it->second.endlessHighScores[difficulty];
        if (score > currentBest) {
            it->second.endlessHighScores[difficulty] = score;
            LOG_DEBUG("New endless high score for " + it->second.username + " (difficulty " +
                      std::to_string(static_cast<int>(difficulty)) + "): " + std::to_string(score));
        }
        save();
    }
}

std::vector<std::pair<std::string, uint32_t>> UserDatabase::getEndlessTopScores(uint8_t difficulty,
                                                                                size_t limit) {
    std::lock_guard<std::mutex> lock(_mutex);

    // Collect all non-guest users with their endless scores for this difficulty
    std::vector<std::pair<std::string, uint32_t>> scores;
    for (const auto &[id, user] : _users) {
        if (!user.isGuest) {
            auto scoreIt = user.endlessHighScores.find(difficulty);
            if (scoreIt != user.endlessHighScores.end() && scoreIt->second > 0) {
                scores.emplace_back(user.username, scoreIt->second);
            }
        }
    }

    // Sort by score descending
    std::sort(scores.begin(), scores.end(),
              [](const auto &a, const auto &b) { return a.second > b.second; });

    // Limit results
    if (scores.size() > limit) {
        scores.resize(limit);
    }

    return scores;
}

// Admin ban methods

void UserDatabase::banUser(uint32_t userId) {
    std::lock_guard<std::mutex> lock(_mutex);
    _bannedUserIds.insert(userId);
    std::cout << "[UserDatabase] Banned user ID: " << userId << std::endl;
}

bool UserDatabase::unbanUser(uint32_t userId) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _bannedUserIds.find(userId);
    if (it != _bannedUserIds.end()) {
        _bannedUserIds.erase(it);
        std::cout << "[UserDatabase] Unbanned user ID: " << userId << std::endl;
        return true;
    }
    return false;
}

bool UserDatabase::isUserBanned(uint32_t userId) const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _bannedUserIds.count(userId) > 0;
}

std::set<uint32_t> UserDatabase::getBannedUserIds() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _bannedUserIds;
}

std::string UserDatabase::getUsername(uint32_t userId) const {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _users.find(userId);
    if (it != _users.end()) {
        return it->second.username;
    }
    return "Unknown";
}
