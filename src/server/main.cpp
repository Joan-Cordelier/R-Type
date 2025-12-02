/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** main
*/

#include "Session/SessionManager.hpp"
#include "Handler/MessageHandler.hpp"
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
    MessageHandler handler(session, running);
    
    session.start();

    LOG_INFO("Server started - all systems running");

    handler.processMessages();

    LOG_INFO("Shutting down server...");
    session.stop();
    LOG_INFO("Server stopped");

    return 0;
}
