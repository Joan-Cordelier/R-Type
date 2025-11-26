/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** MessageFactory
*/

#include "MessageFactory.hpp"

DecodedMessage MessageFactory::decode(const std::vector<uint8_t>& rawData)
{
    DecodedMessage message;
    
    if (rawData.empty()) {
        message.opCode = PARSING_ERROR;
        message.len = 0;
        message.priority = Priority::ERROR;
        return message;
    }

    message.opCode = static_cast<OpCode>(rawData[0]);
    
    int expectedLen = MessageFactory::MessageTable[message.opCode].len;
    message.priority = MessageFactory::MessageTable[message.opCode].priority;
    
    size_t headerSize = 1;

    if (expectedLen == MessageFactory::VARIABLE_LEN) {
        if (rawData.size() < 2) {
            message.opCode = PARSING_ERROR;
            message.len = 0;
            message.priority = Priority::ERROR;
            return message;
        }
        message.len = rawData[1];
        headerSize = 2;
    } else {
        message.len = static_cast<uint8_t>(expectedLen);
    }
    
    if (rawData.size() < headerSize + message.len) {
        message.opCode = PARSING_ERROR;
        message.len = 0;
        message.priority = Priority::ERROR;
        return message;
    }

    if (message.len > 0) {
        message.data.assign(rawData.begin() + headerSize, 
                           rawData.begin() + headerSize + message.len);
    }
    
    return message;
}
