/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** main
*/

#include "Admin/AdminConsole.hpp"
#include "Lobby/LobbyManager.hpp"
#include "Logs/Logger.hpp"
#include "Monitoring/PrometheusExporter.hpp"
#include <atomic>
#include <csignal>
#include <cstring>

std::atomic<bool> running(true);

// We need a pointer to lobbyManager for signal handler if we want to stop it
// gracefull via atomic? Or just let atomic bool handle it if passed?
// LobbyManager has _running atomic member.

// Actually main() has `running`. LobbyManager has its own.
// We should probably just call lobby.stop() in signal handler?
// But signal handler is static.

// Let's keep it simple: atomic global running flag.
// Pass it to LobbyManager?
// LobbyManager has `_session`. SessionManager has `start` and `stop`.
// LobbyManager::run() blocks until stopped.

LobbyManager *g_lobby = nullptr;

void signalHandler(int sig) {
    (void)sig;
    LOG_INFO("Received shutdown signal");
    if (g_lobby) {
        g_lobby->stop();
    }
}

void showHelp() {
    std::cout << "Usage: r-type_server [options]\n"
              << "Options:\n"
              << "  --admin, -a    Start in admin mode (quiet - only errors shown)\n"
              << "  --help, -h     Show this help message\n";
}

int main(int argc, char *argv[]) {
    bool adminMode = false;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--admin") == 0 || std::strcmp(argv[i], "-a") == 0) {
            adminMode = true;
        } else if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            showHelp();
            return 0;
        }
    }

    // Enable quiet mode if admin mode
    if (adminMode) {
        Logger::getInstance().setQuiet(true);
    }

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    LOG_INFO("Starting R-Type server...");

    try {
        PrometheusExporter monitor("0.0.0.0:8080");
        LOG_INFO("Monitoring started on port 8080");
        LobbyManager lobby("yaml/main_loop.yaml", monitor);
        g_lobby = &lobby;
        AdminConsole admin(lobby);

        if (adminMode) {
            admin.start();
            std::cout << "[Admin Mode] Logs suppressed. Type 'verbose' to enable." << std::endl;
        }

        LOG_INFO("Server started - all systems running");
        lobby.run();

        if (adminMode) {
            admin.stop();
        }

        LOG_INFO("Shutting down server...");

    } catch (const std::exception &e) {
        std::cerr << "[CRITICAL ERROR] " << e.what() << std::endl;
        return 84;
    }

    return 0;
}
