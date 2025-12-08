#include "GameHandler.hpp"
#include "../Logs/Logger.hpp"

#include <chrono>
#include <thread>

GameHandler::GameHandler(SessionManager& session, std::atomic<bool>& running)
    : _running(running)
    , _session(session)
    , _messageHandler(session, running)
{
    // Set up message handler callbacks
    _messageHandler.setOnPlayerConnect([this](const Player& player) {
        onPlayerConnect(player);
    });
    
    // Set up session manager callback for disconnections
    _session.setOnPlayerDisconnect([this](const Player& player) {
        onPlayerDisconnect(player);
    });
    
    ScoreEntity = reg.createEntity();
    reg.addComponent<Position>(ScoreEntity, 0.f, 0.f);
    reg.addComponent<Label>(ScoreEntity, std::string("Score: 0"), std::string("font/josefin-sans/JosefinSans-Regular.ttf"), std::string("default_font"), Color(255, 255, 255), 0, true);
}

void GameHandler::run()
{
    LOG_INFO("Game loop started");
    
    auto lastTime = std::chrono::steady_clock::now();
    constexpr float targetFrameTime = 1.0f / 60.0f; // 60 ticks per second

    while (_running) {
        auto currentTime = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Process all pending messages
        processMessages();
        
        // Update game state
        updateGame(deltaTime);

        // Frame rate limiting
        auto frameEnd = std::chrono::steady_clock::now();
        float frameTime = std::chrono::duration<float>(frameEnd - currentTime).count();
        if (frameTime < targetFrameTime) {
            std::this_thread::sleep_for(
                std::chrono::duration<float>(targetFrameTime - frameTime)
            );
        }
    }
    
    LOG_INFO("Game loop stopped");
}

void GameHandler::processMessages()
{
    // Process up to a maximum number of messages per frame to avoid starvation
    constexpr int maxMessagesPerFrame = 100;
    int processed = 0;
    
    while (processed < maxMessagesPerFrame && _messageHandler.processSingleCycle()) {
        processed++;
    }
}

void GameHandler::updateGame(float deltaTime)
{
    movement.update(reg, deltaTime);
    statsys.update(reg, deltaTime);
}

void GameHandler::onPlayerConnect(const Player& player)
{
    Entity playerEntity = reg.createEntity();
    reg.addComponent<Position>(playerEntity, 0.f, 0.f);
    reg.addComponent<Velocity>(playerEntity, 0.f, 0.f);
    reg.addComponent<Stats>(playerEntity, 100, 100, 1, 0.f, 10, 1, 200);
    
    playerEntities[player.id] = playerEntity;
    LOG_INFO("Player " + std::to_string(player.id) + " entity created");
}

void GameHandler::onPlayerDisconnect(const Player& player)
{
    auto it = playerEntities.find(player.id);
    if (it != playerEntities.end()) {
        reg.destroyEntity(it->second);
        playerEntities.erase(it);
        LOG_INFO("Player " + std::to_string(player.id) + " entity destroyed");
    }
}