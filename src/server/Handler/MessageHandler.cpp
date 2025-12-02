/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** MessageHandler
*/

#include "MessageHandler.hpp"
#include "../Logs/Logger.hpp"
#include <thread>
#include <chrono>

MessageHandler::MessageHandler(SessionManager& session, std::atomic<bool>& running)
    : _session(session)
    , _running(running)
{
}

void MessageHandler::processMessages()
{
    while (_running) {
        if (!processSingleCycle()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

bool MessageHandler::processSingleCycle()
{
    auto critical = _session.popMessage(Priority::CRITICAL);
    if (critical.has_value()) {
        dispatchMessage(critical.value());
        return true;
    }
    
    auto high = _session.popMessage(Priority::HIGH);
    if (high.has_value()) {
        dispatchMessage(high.value());
        return true;
    }
    
    auto medium = _session.popMessage(Priority::MEDIUM);
    if (medium.has_value()) {
        dispatchMessage(medium.value());
        return true;
    }
    
    auto low = _session.popMessage(Priority::LOW);
    if (low.has_value()) {
        dispatchMessage(low.value());
        return true;
    }
    
    return false;
}

void MessageHandler::dispatchMessage(const DecodedMessage& msg)
{
    LOG_DEBUG("Dispatching message: OpCode=" + std::to_string(msg.opCode));
    
    switch (msg.opCode) {
        case DEATH:
            handleDeath(msg);
            break;
        case MOVE:
            handleMove(msg);
            break;
        case SHOOT:
            handleShoot(msg);
            break;
        default:
            LOG_WARN("Unknown OpCode: " + std::to_string(msg.opCode));
            break;
    }
}

void MessageHandler::handleDeath(const DecodedMessage& msg)
{
    LOG_INFO("Handling DEATH message");
    // TODO: Implement death logic
    // - Notify ECS to mark entity as dead
    // - Broadcast death to other players
    (void)msg;
}

void MessageHandler::handleMove(const DecodedMessage& msg)
{
    LOG_DEBUG("Handling MOVE message");
    // TODO: Implement move logic
    // - Parse movement data from msg.data
    // - Update ECS entity position
    // - Broadcast position to other players
    (void)msg;
}

void MessageHandler::handleShoot(const DecodedMessage& msg)
{
    LOG_DEBUG("Handling SHOOT message");
    // TODO: Implement shoot logic
    // - Parse shoot data from msg.data
    // - Create bullet entity in ECS
    // - Broadcast bullet spawn to other players
    (void)msg;
}
