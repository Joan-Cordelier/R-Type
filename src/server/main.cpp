/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** main - Server entry point with lobby and room architecture
*/

#include "Lobby/LobbyHandler.hpp"
#include "Logs/Logger.hpp"
#include "Room/RoomManager.hpp"
#include "Session/SessionManager.hpp"
#include <atomic>
#include <chrono>
#include <csignal>
#include <thread>

std::atomic<bool> running(true);

void signalHandler(int sig) {
    (void)sig;
    LOG_INFO("Received shutdown signal");
    running = false;
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    LOG_INFO("Starting R-Type server...");

    // Create lobby session (TCP only on fixed port 4789 for connection/room
    // management)
    SessionManager lobbySession(SessionConfig::lobby());
    RoomManager roomManager(lobbySession);
    LobbyHandler lobbyHandler(lobbySession, roomManager);

    // Create default room (has its own TCP+UDP on dynamic ports)
    Room *defaultRoom = roomManager.createRoom("Default Room");
    if (defaultRoom) {
        LOG_INFO("Default room created - TCP:" +
                 std::to_string(defaultRoom->getTcpPort()) +
                 " UDP:" + std::to_string(defaultRoom->getUdpPort()));
    }

    // Start lobby session
    lobbySession.start();

    LOG_INFO("Server started - Lobby on TCP:4789");
    LOG_INFO("Players connect to lobby, get room ports, then connect to room");

    // Main loop - process lobby messages via LobbyHandler
    while (running) {
        for (Priority prio : {Priority::CRITICAL, Priority::HIGH,
                              Priority::MEDIUM, Priority::LOW}) {
            auto msg = lobbySession.popMessage(prio);
            if (msg.has_value()) {
                lobbyHandler.handleMessage(*msg);
            }
        }

        // Sleep briefly to avoid busy-waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    LOG_INFO("Shutting down server...");
    lobbySession.stop();
    LOG_INFO("Server stopped");

    return 0;
}
