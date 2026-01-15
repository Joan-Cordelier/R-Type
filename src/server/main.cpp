/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** main
*/

#include "Lobby/LobbyManager.hpp"
#include "Logs/Logger.hpp"
#include "Monitoring/PrometheusExporter.hpp"
#include <atomic>
#include <csignal>

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

int main() {
  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);

  LOG_INFO("Starting R-Type server...");

  try {
      PrometheusExporter monitor("0.0.0.0:8080");
      LOG_INFO("Monitoring started on port 8080");
      LobbyManager lobby("yaml/main_loop.yaml", monitor); 
      g_lobby = &lobby;
      LOG_INFO("Server started - all systems running");
      lobby.run();
      LOG_INFO("Shutting down server...");

  } catch (const std::exception &e) {
      std::cerr << "[CRITICAL ERROR] " << e.what() << std::endl;
      return 84;
  }

  return 0;
}
