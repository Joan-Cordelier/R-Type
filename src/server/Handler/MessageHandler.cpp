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
#include <cstring>

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

void MessageHandler::dispatchMessage(DecodedMessage msg)
{
    LOG_DEBUG("Dispatching message: OpCode=" + std::to_string(msg.opCode));
    
    if (msg.playerId == 0) {
        if (msg.tcpFd >= 0) {
            auto* player = _session.getPlayerByTcpFd(msg.tcpFd);
            if (player) {
                msg.playerId = player->id;
                LOG_DEBUG("Resolved playerId=" + std::to_string(msg.playerId) + " from tcpFd=" + std::to_string(msg.tcpFd));
            }
        } else if (msg.udpAddr.sin_port != 0) {
            auto* player = _session.getPlayerByUdpAddr(msg.udpAddr);
            if (player) {
                msg.playerId = player->id;
                LOG_DEBUG("Resolved playerId=" + std::to_string(msg.playerId) + " from UDP address");
            } else {
                LOG_WARN("Could not resolve player from UDP address");
            }
        }
    }
    
    switch (msg.opCode) {
        case CONNECT:
            handleConnect(msg);
            break;
        case LINK:
            handleLink(msg);
            break;
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

void MessageHandler::handleLink(const DecodedMessage& msg)
{
    // LINK message: client sends playerId via UDP to link UDP address
    if (msg.data.size() < 4) {
        LOG_WARN("LINK message with invalid payload size: " + std::to_string(msg.data.size()));
        return;
    }
    
    // Extract playerId from payload (4 bytes, big-endian)
    uint32_t playerId = 
        (static_cast<uint32_t>(msg.data[0]) << 24) |
        (static_cast<uint32_t>(msg.data[1]) << 16) |
        (static_cast<uint32_t>(msg.data[2]) << 8) |
        static_cast<uint32_t>(msg.data[3]);
    
    LOG_INFO("Handling LINK message: playerId=" + std::to_string(playerId));
    
    // Verify player exists
    auto* player = _session.getPlayer(playerId);
    if (!player) {
        LOG_WARN("LINK message for unknown playerId: " + std::to_string(playerId));
        return;
    }
    
    // Link UDP address to player
    if (msg.udpAddr.sin_port != 0) {
        _session.linkPlayerUdp(playerId, msg.udpAddr);
        LOG_INFO("Player " + std::to_string(playerId) + " UDP linked successfully");
    } else {
        LOG_WARN("LINK message without valid UDP address");
    }
}

void MessageHandler::handleConnect(const DecodedMessage& msg)
{
    LOG_INFO("Handling CONNECT message (fd: " + std::to_string(msg.tcpFd) + ", playerId: " + std::to_string(msg.playerId) + ")");
    
    // Player should already be registered when TCP connection was established
    auto* player = _session.getPlayerByTcpFd(msg.tcpFd);
    if (!player) {
        LOG_ERROR("CONNECT message from unknown fd: " + std::to_string(msg.tcpFd));
        return;
    }
    
    uint32_t playerId = player->id;
    
    // Mark player as connected/ready
    player->connected = true;
    
    // Notify GameHandler to create player entity
    if (_onPlayerConnect) {
        _onPlayerConnect(*player);
    }
    
    // Send confirmation back to the player with their assigned player ID
    auto& factory = MessageFactory::getInstance();
    std::vector<uint8_t> payload;
    payload.push_back(static_cast<uint8_t>((playerId >> 24) & 0xFF));
    payload.push_back(static_cast<uint8_t>((playerId >> 16) & 0xFF));
    payload.push_back(static_cast<uint8_t>((playerId >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(playerId & 0xFF));
    
    auto response = factory.createMessage(CONNECT_ACK, payload);
    _session.sendTcp(playerId, response);
    
    // Send JOIN message with the room ID
    uint8_t roomId = player->roomId;
    std::vector<uint8_t> joinPayload;
    joinPayload.push_back(roomId);
    
    auto joinMsg = factory.createMessage(JOIN, joinPayload);
    _session.sendTcp(playerId, joinMsg);
    
    LOG_INFO("Player " + std::to_string(playerId) + " connected successfully, joined room " + std::to_string(roomId));
}

void MessageHandler::handleMove(const DecodedMessage& msg)
{
    LOG_DEBUG("Handling MOVE message");
    
    if (msg.data.size() < 12) {
        LOG_WARN("MOVE message with invalid payload size: " + std::to_string(msg.data.size()));
        return;
    }
    
    // Extract velocity x (4 bytes, float)
    float vx;
    std::memcpy(&vx, &msg.data[4], sizeof(float));
    
    // Extract velocity y (4 bytes, float)
    float vy;
    std::memcpy(&vy, &msg.data[8], sizeof(float));
    
    LOG_DEBUG("MOVE: playerId=" + std::to_string(msg.playerId) + " vx=" + std::to_string(vx) + " vy=" + std::to_string(vy));
    
    if (_onPlayerMove) {
        MoveData moveData{msg.playerId, vx, vy};
        _onPlayerMove(moveData);
    }
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
