#include "GameHandler.hpp"

#include <chrono>

GameHandler::GameHandler(MessageHandler handler) : _gameStarted(false), _running(true), messageHandler(handler)
{
    ScoreEntity = reg.createEntity();
    reg.addComponent<Position>(ScoreEntity, 0.f, 0.f);
    reg.addComponent<Label>(ScoreEntity, "Score: 0", "font/josefin-sans/JosefinSans-Regular.ttf", "default_font", Color(255, 255, 255), 0, true);
}

void GameHandler::gameLoop()
{
    auto lastTime = std::chrono::steady_clock::now();

    while (_running) {
        auto currentTime = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        movement.update(reg, deltaTime);
        statsys.update(reg, deltaTime);

        messageHandler.processMessages();
    }
}

void GameHandler::onPlayerConnect(Player player)
{
    Entity playerEntity = reg.createEntity();
    reg.addComponent<Position>(playerEntity, 0.f, 0.f);
    reg.addComponent<Velocity>(playerEntity, 0.f, 0.f);
    reg.addComponent<Stats>(playerEntity, 100, 100, 1, 0.f, 10, 1, 200);
}

void GameHandler::onPlayerDisconnect(Player player)
{
    auto it = playerEntities.find(player.id);
    if (it != playerEntities.end()) {
        reg.destroyEntity(it->second);
        playerEntities.erase(it);
    }
}