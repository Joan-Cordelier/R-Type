/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** main
*/

#include "Session/SessionManager.hpp"
#include "Gameplay/GameHandler.hpp"
#include "Logs/Logger.hpp"
#include <csignal>
#include <atomic>

std::atomic<bool> running(true);

void signalHandler(int sig)
{
    (void)sig;
    LOG_INFO("Received shutdown signal");
    running = false;
}

int main()
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    LOG_INFO("Starting R-Type server...");

    SessionManager session;
    GameHandler game(session, running, "yaml/main_loop.yaml");
    
    session.start();

    LOG_INFO("Server started - all systems running");

    // Run the main game loop
    game.run();

    LOG_INFO("Shutting down server...");
    session.stop();
    LOG_INFO("Server stopped");

    return 0;
}
