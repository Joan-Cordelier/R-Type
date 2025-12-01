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

class MessageHandler {
    public:
        MessageHandler(SessionManager& session, std::atomic<bool>& running);
        ~MessageHandler() = default;
        
        // Main processing loop
        void processMessages();
        
        // Process a single message cycle (returns true if message was processed)
        bool processSingleCycle();

    private:
        SessionManager& _session;
        std::atomic<bool>& _running;
        
        // Dispatch message to appropriate handler
        void dispatchMessage(const DecodedMessage& msg);
        
        // Message handlers by OpCode
        void handleDeath(const DecodedMessage& msg);
        void handleMove(const DecodedMessage& msg);
        void handleShoot(const DecodedMessage& msg);
};

#endif /* !MESSAGEHANDLER_HPP_ */
