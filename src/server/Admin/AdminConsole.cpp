/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** AdminConsole - Text-based admin interface implementation
*/

#include "AdminConsole.hpp"
#include "../Lobby/LobbyManager.hpp"
#include "../Logs/Logger.hpp"
#include <iomanip>

AdminConsole::AdminConsole(LobbyManager &lobby) : _lobby(lobby) {
    _startTime = std::chrono::steady_clock::now();

    // Register commands
    _commands["help"] = [this](const auto &) { cmdHelp(); };
    _commands["rooms"] = [this](const auto &) { cmdRooms(); };
    _commands["users"] = [this](const auto &) { cmdUsers(); };
    _commands["kick"] = [this](const auto &args) { cmdKick(args); };
    _commands["kickuser"] = [this](const auto &args) { cmdKickByUserId(args); };
    _commands["ban"] = [this](const auto &args) { cmdBan(args); };
    _commands["banuser"] = [this](const auto &args) { cmdBanByUserId(args); };
    _commands["unban"] = [this](const auto &args) { cmdUnban(args); };
    _commands["banlist"] = [this](const auto &) { cmdBanList(); };
    _commands["stats"] = [this](const auto &) { cmdStats(); };
    _commands["netstats"] = [this](const auto &) { cmdNetstats(); };
    _commands["quiet"] = [this](const auto &) { cmdQuiet(); };
    _commands["verbose"] = [this](const auto &) { cmdVerbose(); };
    _commands["quit"] = [this](const auto &) { cmdQuit(); };
    _commands["exit"] = [this](const auto &) { cmdQuit(); };
}

AdminConsole::~AdminConsole() {
    stop();
}

void AdminConsole::start() {
    if (_running.exchange(true)) {
        return; // Already running
    }
    _thread = std::thread(&AdminConsole::run, this);
    LOG_INFO("[AdminConsole] Started - type 'help' for commands");
}

void AdminConsole::stop() {
    _running = false;
    if (_thread.joinable()) {
        _thread.join();
    }
}

void AdminConsole::run() {
    std::cout << "\n=== R-Type Server Admin Console ===" << std::endl;
    std::cout << "Type 'help' for available commands\n" << std::endl;

    std::string line;
    while (_running) {
        std::cout << "admin> " << std::flush;

        if (!std::getline(std::cin, line)) {
            // EOF or error
            if (_running) {
                std::cout << "\n[AdminConsole] Input closed, shutting down..." << std::endl;
                _lobby.stop();
            }
            break;
        }

        if (line.empty()) {
            continue;
        }

        processCommand(line);
    }
}

std::vector<std::string> AdminConsole::parseArgs(const std::string &input) {
    std::vector<std::string> args;
    std::istringstream iss(input);
    std::string token;

    while (iss >> token) {
        args.push_back(token);
    }
    return args;
}

void AdminConsole::processCommand(const std::string &input) {
    auto args = parseArgs(input);
    if (args.empty()) {
        return;
    }

    std::string cmd = args[0];
    // Convert to lowercase
    for (auto &c : cmd) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    auto it = _commands.find(cmd);
    if (it != _commands.end()) {
        it->second(args);
    } else {
        std::cout << "Unknown command: " << cmd << ". Type 'help' for available commands."
                  << std::endl;
    }
}

void AdminConsole::cmdHelp() {
    std::cout << "\n=== Available Commands ===" << std::endl;
    std::cout << "  help             - Show this help message" << std::endl;
    std::cout << "  rooms            - List all active game rooms" << std::endl;
    std::cout << "  users            - List all connected users" << std::endl;
    std::cout << "  kick <playerID>  - Kick by PlayerID (first column)" << std::endl;
    std::cout << "  kickuser <userID>- Kick by UserID (second column)" << std::endl;
    std::cout << "  ban <playerID>   - Ban by PlayerID" << std::endl;
    std::cout << "  banuser <userID> - Ban by UserID" << std::endl;
    std::cout << "  unban <userID>   - Unban by UserID" << std::endl;
    std::cout << "  stats            - Show server statistics" << std::endl;
    std::cout << "  netstats         - Show network bandwidth/message stats" << std::endl;
    std::cout << "  quiet            - Disable log output (only errors)" << std::endl;
    std::cout << "  verbose          - Enable all log output" << std::endl;
    std::cout << "  quit/exit        - Shutdown the server" << std::endl;
    std::cout << std::endl;
}

void AdminConsole::cmdRooms() {
    auto rooms = _lobby.getAdminRoomList();

    if (rooms.empty()) {
        std::cout << "No active rooms." << std::endl;
        return;
    }

    std::cout << "\n=== Active Rooms (" << rooms.size() << ") ===" << std::endl;
    std::cout << std::left << std::setw(8) << "ID" << std::setw(10) << "Players" << std::setw(15)
              << "Game Mode" << std::setw(12) << "Difficulty" << std::endl;
    std::cout << std::string(45, '-') << std::endl;

    for (const auto &room : rooms) {
        std::cout << std::left << std::setw(8) << room.id << std::setw(10) << room.playerCount
                  << std::setw(15) << room.gameMode << std::setw(12) << room.difficulty
                  << std::endl;
    }
    std::cout << std::endl;
}

void AdminConsole::cmdUsers() {
    auto users = _lobby.getAdminUserList();

    if (users.empty()) {
        std::cout << "No connected users." << std::endl;
        return;
    }

    std::cout << "\n=== Connected Users (" << users.size() << ") ===" << std::endl;
    std::cout << std::left << std::setw(10) << "PlayerID" << std::setw(10) << "UserID"
              << std::setw(20) << "Username" << std::setw(10) << "RoomID" << std::setw(8) << "Guest"
              << std::endl;
    std::cout << std::string(58, '-') << std::endl;

    for (const auto &user : users) {
        std::cout << std::left << std::setw(10) << user.playerId << std::setw(10) << user.userId
                  << std::setw(20) << user.username << std::setw(10)
                  << (user.roomId > 0 ? std::to_string(user.roomId) : "-") << std::setw(8)
                  << (user.isGuest ? "Yes" : "No") << std::endl;
    }
    std::cout << std::endl;
}

void AdminConsole::cmdKick(const std::vector<std::string> &args) {
    if (args.size() < 2) {
        std::cout << "Usage: kick <player_id>" << std::endl;
        return;
    }

    try {
        uint32_t playerId = static_cast<uint32_t>(std::stoul(args[1]));
        if (_lobby.kickUser(playerId)) {
            std::cout << "Player " << playerId << " has been kicked." << std::endl;
            LOG_INFO("[AdminConsole] Kicked player " + std::to_string(playerId));
        } else {
            std::cout << "Failed to kick player " << playerId
                      << " (not found or already disconnected)." << std::endl;
        }
    } catch (const std::exception &e) {
        std::cout << "Invalid player ID: " << args[1] << std::endl;
    }
}

void AdminConsole::cmdBan(const std::vector<std::string> &args) {
    if (args.size() < 2) {
        std::cout << "Usage: ban <player_id>" << std::endl;
        return;
    }

    try {
        uint32_t playerId = static_cast<uint32_t>(std::stoul(args[1]));
        if (_lobby.banUser(playerId)) {
            std::cout << "Player " << playerId << " has been banned." << std::endl;
            LOG_INFO("[AdminConsole] Banned player " + std::to_string(playerId));
        } else {
            std::cout << "Failed to ban player " << playerId << " (not found)." << std::endl;
        }
    } catch (const std::exception &e) {
        std::cout << "Invalid player ID: " << args[1] << std::endl;
    }
}

void AdminConsole::cmdKickByUserId(const std::vector<std::string> &args) {
    if (args.size() < 2) {
        std::cout << "Usage: kickuser <user_id>" << std::endl;
        return;
    }

    try {
        uint32_t userId = static_cast<uint32_t>(std::stoul(args[1]));
        // Find player with this userId
        auto users = _lobby.getAdminUserList();
        for (const auto &user : users) {
            if (user.userId == userId) {
                if (_lobby.kickUser(user.playerId)) {
                    std::cout << "User " << userId << " (player " << user.playerId
                              << ") has been kicked." << std::endl;
                    return;
                }
            }
        }
        std::cout << "No connected player with UserID " << userId << " found." << std::endl;
    } catch (const std::exception &e) {
        std::cout << "Invalid user ID: " << args[1] << std::endl;
    }
}

void AdminConsole::cmdBanByUserId(const std::vector<std::string> &args) {
    if (args.size() < 2) {
        std::cout << "Usage: banuser <user_id>" << std::endl;
        return;
    }

    try {
        uint32_t userId = static_cast<uint32_t>(std::stoul(args[1]));
        // Find player with this userId
        auto users = _lobby.getAdminUserList();
        for (const auto &user : users) {
            if (user.userId == userId) {
                if (_lobby.banUser(user.playerId)) {
                    std::cout << "User " << userId << " (player " << user.playerId
                              << ") has been banned." << std::endl;
                    return;
                }
            }
        }
        std::cout << "No connected player with UserID " << userId << " found." << std::endl;
    } catch (const std::exception &e) {
        std::cout << "Invalid user ID: " << args[1] << std::endl;
    }
}

void AdminConsole::cmdUnban(const std::vector<std::string> &args) {
    if (args.size() < 2) {
        std::cout << "Usage: unban <user_id>" << std::endl;
        return;
    }

    try {
        uint32_t userId = static_cast<uint32_t>(std::stoul(args[1]));
        if (_lobby.unbanUser(userId)) {
            std::cout << "User " << userId << " has been unbanned." << std::endl;
            LOG_INFO("[AdminConsole] Unbanned user " + std::to_string(userId));
        } else {
            std::cout << "Failed to unban user " << userId << "." << std::endl;
        }
    } catch (const std::exception &e) {
        std::cout << "Invalid user ID: " << args[1] << std::endl;
    }
}

void AdminConsole::cmdBanList() {
    auto bans = _lobby.getBannedUsers();
    std::cout << "\n=== Banned Users ===" << std::endl;
    std::cout << std::setw(10) << "User ID" << std::setw(20) << "Username" << std::endl;
    for (const auto &ban : bans) {
        std::cout << std::setw(10) << ban.userId << std::setw(20) << ban.username << std::endl;
    }
    std::cout << std::endl;
}

void AdminConsole::cmdStats() {
    auto now = std::chrono::steady_clock::now();
    auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - _startTime);

    auto rooms = _lobby.getAdminRoomList();
    auto users = _lobby.getAdminUserList();
    size_t totalConnections = _lobby.getTotalConnections();

    int hours = static_cast<int>(uptime.count() / 3600);
    int minutes = static_cast<int>((uptime.count() % 3600) / 60);
    int seconds = static_cast<int>(uptime.count() % 60);

    std::cout << "\n=== Server Statistics ===" << std::endl;
    std::cout << "  Uptime:             " << hours << "h " << minutes << "m " << seconds << "s"
              << std::endl;
    std::cout << "  Active Rooms:       " << rooms.size() << std::endl;
    std::cout << "  Connected Users:    " << users.size() << std::endl;
    std::cout << "  Total Connections:  " << totalConnections << std::endl;
    std::cout << std::endl;
}

void AdminConsole::cmdQuiet() {
    Logger::getInstance().setQuiet(true);
    std::cout << "Quiet mode enabled - only errors will be shown." << std::endl;
}

void AdminConsole::cmdVerbose() {
    Logger::getInstance().setQuiet(false);
    std::cout << "Verbose mode enabled - all logs will be shown." << std::endl;
}

void AdminConsole::cmdQuit() {
    std::cout << "Shutting down server..." << std::endl;
    LOG_INFO("[AdminConsole] Server shutdown requested");
    _running = false;
    _lobby.stop();
}

void AdminConsole::cmdNetstats() {
    auto stats = _lobby.getNetworkStats();

    auto formatBytes = [](double bytes) -> std::string {
        if (bytes >= 1024 * 1024) {
            return std::to_string(static_cast<int>(bytes / (1024 * 1024))) + " MB";
        } else if (bytes >= 1024) {
            return std::to_string(static_cast<int>(bytes / 1024)) + " KB";
        }
        return std::to_string(static_cast<int>(bytes)) + " B";
    };

    std::cout << "\n=== Network Statistics ===" << std::endl;
    std::cout << "  TCP Sent:      " << formatBytes(stats.tcpBytesSent) << std::endl;
    std::cout << "  TCP Received:  " << formatBytes(stats.tcpBytesReceived) << std::endl;
    std::cout << "  UDP Sent:      " << formatBytes(stats.udpBytesSent) << std::endl;
    std::cout << "  UDP Received:  " << formatBytes(stats.udpBytesReceived) << std::endl;
    std::cout << std::endl;
    std::cout << "  Messages Sent:     " << stats.totalMessagesSent << std::endl;
    std::cout << "  Messages Received: " << stats.totalMessagesReceived << std::endl;

    if (!stats.messageCountsByType.empty()) {
        std::cout << "\n  === Messages by Type ===" << std::endl;
        for (const auto &[type, count] : stats.messageCountsByType) {
            std::cout << "    " << std::setw(20) << std::left << type << count << std::endl;
        }
    }
    std::cout << std::endl;
}
