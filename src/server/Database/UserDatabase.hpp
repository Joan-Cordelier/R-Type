/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** UserDatabase - JSON-based user storage with authentication
*/

#ifndef USERDATABASE_HPP_
#define USERDATABASE_HPP_

#include <string>
#include <optional>
#include <unordered_map>
#include <mutex>
#include <cstdint>

struct User {
    uint32_t id;
    std::string username;
    std::string passwordHash;
    uint64_t createdAt;      // Unix timestamp
    uint64_t lastLogin;      // Unix timestamp
    bool isGuest;
};

class UserDatabase {
public:
    static UserDatabase& getInstance();

    // Disable copy
    UserDatabase(const UserDatabase&) = delete;
    UserDatabase& operator=(const UserDatabase&) = delete;

    uint32_t registerUser(const std::string& username, const std::string& password);
    std::optional<User> login(const std::string& username, const std::string& password);
    User createGuest();

    std::optional<User> getUserById(uint32_t userId);
    std::optional<User> getUserByUsername(const std::string& username);

    bool usernameExists(const std::string& username);
    bool isGuest(uint32_t userId);

    void updateLastLogin(uint32_t userId);
    void save();
    void load();

private:
    UserDatabase();
    ~UserDatabase() = default;

    std::string hashPassword(const std::string& password);
    uint32_t generateUserId();
    std::string generateGuestUsername();

    bool isValidUsername(const std::string& username);

    std::unordered_map<uint32_t, User> _users;
    std::unordered_map<std::string, uint32_t> _usernameIndex;
    std::mutex _mutex;
    uint32_t _nextUserId = 1;
    uint32_t _nextGuestNumber = 1;  //(Guest_1, Guest_2, ...)
    std::string _filePath = "data/users.json";
};

#endif // USERDATABASE_HPP_
