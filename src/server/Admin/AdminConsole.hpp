/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** AdminConsole - Text-based admin interface for server management
*/

#ifndef ADMIN_CONSOLE_HPP
#define ADMIN_CONSOLE_HPP

#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

class LobbyManager;

class AdminConsole {
public:
    AdminConsole(LobbyManager &lobby);
    ~AdminConsole();

    void start();
    void stop();

private:
    void run();
    void processCommand(const std::string &input);
    std::vector<std::string> parseArgs(const std::string &input);

    // Commands
    void cmdHelp();
    void cmdRooms();
    void cmdUsers();
    void cmdKick(const std::vector<std::string> &args);
    void cmdKickByUserId(const std::vector<std::string> &args);
    void cmdBan(const std::vector<std::string> &args);
    void cmdBanByUserId(const std::vector<std::string> &args);
    void cmdUnban(const std::vector<std::string> &args);
    void cmdBanList();
    void cmdStats();
    void cmdNetstats();
    void cmdConnections();
    void cmdQuiet();
    void cmdVerbose();
    void cmdQuit();

    LobbyManager &_lobby;
    std::atomic<bool> _running{false};
    std::thread _thread;
    std::chrono::steady_clock::time_point _startTime;

    using CommandHandler = std::function<void(const std::vector<std::string> &)>;
    std::map<std::string, CommandHandler> _commands;
};

#endif /* !ADMIN_CONSOLE_HPP */
