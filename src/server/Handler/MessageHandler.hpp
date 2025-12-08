/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** MessageHandler
*/

#ifndef MESSAGEHANDLER_HPP_
#define MESSAGEHANDLER_HPP_

#include "../Session/SessionManager.hpp"
#include "../../common/Data/MessageFactory.hpp"
#include <atomic>
#include <functional>

class MessageHandler {
    public:
        using PlayerCallback = std::function<void(const Player&)>;
        
        MessageHandler(SessionManager& session, std::atomic<bool>& running);
        ~MessageHandler() = default;
        
        void processMessages();
        
        bool processSingleCycle();
        
        void setOnPlayerConnect(PlayerCallback callback) { _onPlayerConnect = callback; }
        void setOnPlayerDisconnect(PlayerCallback callback) { _onPlayerDisconnect = callback; }

    private:
        SessionManager& _session;
        std::atomic<bool>& _running;
        
        PlayerCallback _onPlayerConnect;
        PlayerCallback _onPlayerDisconnect;
        
        void dispatchMessage(DecodedMessage msg);
        
        void handleConnect(const DecodedMessage& msg);
        void handleLink(const DecodedMessage& msg);
        void handleDeath(const DecodedMessage& msg);
        void handleMove(const DecodedMessage& msg);
        void handleShoot(const DecodedMessage& msg);
};

#endif /* !MESSAGEHANDLER_HPP_ */
