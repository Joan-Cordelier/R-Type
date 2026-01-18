/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** UserDatabase - JSON-based user storage implementation
*/

#include "UserDatabase.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <openssl/sha.h>

namespace {
    std::string sha256(const std::string& input) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);

        std::ostringstream result;
        result << std::hex << std::setfill('0');
        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
            result << std::setw(2) << static_cast<int>(hash[i]);
        }
        return result.str();
    }
}

UserDatabase& UserDatabase::getInstance() {
    static UserDatabase instance;
    return instance;
}

UserDatabase::UserDatabase() {
    load();
}

std::string UserDatabase::hashPassword(const std::string& password) {
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

bool UserDatabase::isValidUsername(const std::string& username) {
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

uint32_t UserDatabase::registerUser(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(_mutex);

    if (!isValidUsername(username)) {
        std::cout << "[UserDB] Registration failed: invalid username '" << username << "'" << std::endl;
        return 0;
    }
    if (_usernameIndex.find(username) != _usernameIndex.end()) {
        std::cout << "[UserDB] Registration failed: username '" << username << "' already exists" << std::endl;
        return 0;
    }
    if (password.length() < 4) {
        std::cout << "[UserDB] Registration failed: password too short" << std::endl;
        return 0;
    }

    User user;
    user.id = generateUserId();
    user.username = username;
    user.passwordHash = hashPassword(password);
    user.createdAt = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    user.lastLogin = user.createdAt;
    user.isGuest = false;

    _users[user.id] = user;
    _usernameIndex[username] = user.id;

    std::cout << "[UserDB] User registered: " << username << " (ID: " << user.id << ")" << std::endl;
    
    save();
    return user.id;
}

std::optional<User> UserDatabase::login(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(_mutex);

    auto it = _usernameIndex.find(username);
    if (it == _usernameIndex.end()) {
        std::cout << "[UserDB] Login failed: user '" << username << "' not found" << std::endl;
        return std::nullopt;
    }

    User& user = _users[it->second];
    
    if (user.passwordHash != hashPassword(password)) {
        std::cout << "[UserDB] Login failed: wrong password for '" << username << "'" << std::endl;
        return std::nullopt;
    }

    user.lastLogin = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    std::cout << "[UserDB] User logged in: " << username << " (ID: " << user.id << ")" << std::endl;
    
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
        std::chrono::system_clock::now().time_since_epoch()).count();
    guest.lastLogin = guest.createdAt;
    guest.isGuest = true;

    _users[guest.id] = guest;
    _usernameIndex[guest.username] = guest.id;

    std::cout << "[UserDB] Guest created: " << guest.username << " (ID: " << guest.id << ")" << std::endl;

    return guest;
}

std::optional<User> UserDatabase::getUserById(uint32_t userId) {
    std::lock_guard<std::mutex> lock(_mutex);
    
    auto it = _users.find(userId);
    if (it != _users.end())
        return it->second;
    return std::nullopt;
}

std::optional<User> UserDatabase::getUserByUsername(const std::string& username) {
    std::lock_guard<std::mutex> lock(_mutex);
    
    auto it = _usernameIndex.find(username);
    if (it != _usernameIndex.end())
        return _users[it->second];
    return std::nullopt;
}

bool UserDatabase::usernameExists(const std::string& username) {
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
            std::chrono::system_clock::now().time_since_epoch()).count();
        save();
    }
}

void UserDatabase::save() {
    // Ensure directory exists
    std::filesystem::create_directories("data");

    std::ofstream file(_filePath);
    if (!file.is_open()) {
        std::cerr << "[UserDB] Failed to save: cannot open " << _filePath << std::endl;
        return;
    }

    file << "{\n";
    file << "  \"nextUserId\": " << _nextUserId << ",\n";
    file << "  \"users\": [\n";

    bool first = true;
    for (const auto& [id, user] : _users) {
        if (user.isGuest)
            continue;

        if (!first) file << ",\n";
        first = false;

        file << "    {\n";
        file << "      \"id\": " << user.id << ",\n";
        file << "      \"username\": \"" << user.username << "\",\n";
        file << "      \"passwordHash\": \"" << user.passwordHash << "\",\n";
        file << "      \"createdAt\": " << user.createdAt << ",\n";
        file << "      \"lastLogin\": " << user.lastLogin << ",\n";
        file << "      \"highScore\": " << user.highScore << ",\n";
        file << "      \"totalGamesPlayed\": " << user.totalGamesPlayed << "\n";
        file << "    }";
    }

    file << "\n  ]\n";
    file << "}\n";

    file.close();
    std::cout << "[UserDB] Database saved to " << _filePath << std::endl;
}

void UserDatabase::load() {
    std::ifstream file(_filePath);
    if (!file.is_open()) {
        std::cout << "[UserDB] No existing database found, starting fresh" << std::endl;
        return;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    // Parse nextUserId
    size_t pos = content.find("\"nextUserId\":");
    if (pos != std::string::npos) {
        pos += 13;
        while (pos < content.size() && (content[pos] == ' ' || content[pos] == '\t')) pos++;
        _nextUserId = std::stoul(content.substr(pos));
    }

    // Parse users array
    pos = content.find("\"users\":");
    if (pos == std::string::npos) return;

    size_t arrayStart = content.find('[', pos);
    size_t arrayEnd = content.rfind(']');
    if (arrayStart == std::string::npos || arrayEnd == std::string::npos) return;

    std::string usersStr = content.substr(arrayStart, arrayEnd - arrayStart + 1);

    // Find each user object
    size_t objStart = 0;
    while ((objStart = usersStr.find('{', objStart)) != std::string::npos) {
        size_t objEnd = usersStr.find('}', objStart);
        if (objEnd == std::string::npos) break;

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

        if (user.id > 0 && !user.username.empty()) {
            _users[user.id] = user;
            _usernameIndex[user.username] = user.id;
            std::cout << "[UserDB] Loaded user: " << user.username << " (ID: " << user.id << ")" << std::endl;
        }

        objStart = objEnd + 1;
    }

    std::cout << "[UserDB] Loaded " << _users.size() << " users from " << _filePath << std::endl;
}

void UserDatabase::updateScore(uint32_t userId, uint32_t score) {
    std::lock_guard<std::mutex> lock(_mutex);
    
    auto it = _users.find(userId);
    if (it != _users.end()) {
        it->second.totalGamesPlayed++;
        if (score > it->second.highScore) {
            it->second.highScore = score;
            std::cout << "[UserDB] New high score for " << it->second.username << ": " << score << std::endl;
        }
        save();
    }
}

std::vector<std::pair<std::string, uint32_t>> UserDatabase::getTopScores(size_t limit) {
    std::lock_guard<std::mutex> lock(_mutex);
    
    // Collect all non-guest users with their scores
    std::vector<std::pair<std::string, uint32_t>> scores;
    for (const auto& [id, user] : _users) {
        if (!user.isGuest && user.highScore > 0) {
            scores.emplace_back(user.username, user.highScore);
        }
    }
    
    // Sort by score descending
    std::sort(scores.begin(), scores.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });
    
    // Limit results
    if (scores.size() > limit) {
        scores.resize(limit);
    }
    
    return scores;
}
