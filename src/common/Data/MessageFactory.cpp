/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** MessageFactory
*/

#include "MessageFactory.hpp"
#include "LinearBuffer.hpp"

MessageFactory::MessageFactory() : _messageTable(initMessageTable())
{
}

std::array<MessageFactory::Message, 256> MessageFactory::initMessageTable()
{
    std::array<Message, 256> table{};

    for (size_t i = 0; i < 256; ++i)
        table[i] = {0, Priority::MEDIUM};

    table[INCOMPLETE] = {0, Priority::ERROR};
    table[PARSING_ERROR] = {0, Priority::ERROR};
    table[DEATH] = {0, Priority::CRITICAL};
    table[SHOOT] = {4, Priority::HIGH};
    table[MOVE] = {12, Priority::LOW};
    table[CONNECT] = {0, Priority::CRITICAL};      // Client request (no payload)
    table[CONNECT_ACK] = {4, Priority::CRITICAL};  // Server response with playerId (4 bytes)
    table[START] = {0, Priority::CRITICAL}; 
    table[JOIN] = {1, Priority::CRITICAL}; 
    table[CRASH] = {1, Priority::CRITICAL}; 
    table[PLAYER] = {16, Priority::CRITICAL}; // playerid (4) + entityId (4) + x (4) + y (4)
    table[LINK] = {4, Priority::CRITICAL};

    return table;
}

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
    
    int expectedLen = _messageTable[message.opCode].len;
    message.priority = _messageTable[message.opCode].priority;
    
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

bool MessageFactory::decodeHeader(uint8_t op, size_t bufferSize, 
                                  uint8_t secondByte, size_t& headerSize, 
                                  uint8_t& payloadLen, Priority& priority) const
{
    int expectedLen = _messageTable[op].len;
    priority = _messageTable[op].priority;
    
    headerSize = 1;
    
    if (expectedLen == VARIABLE_LEN) {
        if (bufferSize < 2) 
            return false;
        payloadLen = secondByte;
        headerSize = 2;
    } else {
        payloadLen = static_cast<uint8_t>(expectedLen);
    }
    
    if (bufferSize < headerSize + payloadLen)
        return false;
    
    return true;
}

template<typename LinearBufferT>
DecodedMessage MessageFactory::decodeFromBuffer(LinearBufferT& buffer)
{
    DecodedMessage message;
    message.opCode = INCOMPLETE;
    
    if (buffer.size() == 0)
        return message;
    
    uint8_t op = buffer.peek(0);
    uint8_t secondByte = buffer.size() >= 2 ? buffer.peek(1) : 0;
    size_t headerSize = 0;
    uint8_t payloadLen = 0;
    
    if (!decodeHeader(op, buffer.size(), secondByte, headerSize, 
                    payloadLen, message.priority))
        return message;
    
    message.opCode = static_cast<OpCode>(op);
    message.len = payloadLen;
    
    buffer.consume(headerSize);
    
    if (payloadLen > 0)
        buffer.read(message.data, payloadLen);
    
    return message;
}

template DecodedMessage MessageFactory::decodeFromBuffer<LinearBuffer>(LinearBuffer& buffer);

PreparedMessage MessageFactory::createMessage(OpCode opCode, const MessageData& payload) const
{
    PreparedMessage prepared;
    
    int expectedLen = _messageTable[opCode].len;
    
    prepared.data.push_back(static_cast<uint8_t>(opCode));
    
    if (expectedLen == VARIABLE_LEN) {
        prepared.data.push_back(static_cast<uint8_t>(payload.size()));
    }
    
    if (!payload.empty()) {
        prepared.data.insert(prepared.data.end(), payload.begin(), payload.end());
    }
    
    prepared.priority = _messageTable[opCode].priority;
    
    return prepared;
}

MessageData MessageFactory::encodeMessagePlayer(Entity entity) const
{
    MessageData data;
    data.push_back(static_cast<uint8_t>((entity >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(entity & 0xFF));
    return data;
}

MessageData MessageFactory::encodeMessageMovementPlayer(Entity entity, float x, float y) const
{
    MessageData data;
    data.push_back(static_cast<uint8_t>((entity >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(entity & 0xFF));
    
    const uint8_t* px = reinterpret_cast<const uint8_t*>(&x);
    const uint8_t* py = reinterpret_cast<const uint8_t*>(&y);
    
    data.insert(data.end(), px, px + sizeof(float));
    data.insert(data.end(), py, py + sizeof(float));
    
    return data;
}

MessageData MessageFactory::encodePlayerInfo(uint32_t playerId, Entity entity, float x, float y) const
{
    MessageData data;
    
    // Player ID (4 bytes, big-endian)
    data.push_back(static_cast<uint8_t>((playerId >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((playerId >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((playerId >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(playerId & 0xFF));
    
    // Entity ID (4 bytes, big-endian)
    data.push_back(static_cast<uint8_t>((entity >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(entity & 0xFF));
    
    // Position X (4 bytes, float)
    const uint8_t* px = reinterpret_cast<const uint8_t*>(&x);
    data.insert(data.end(), px, px + sizeof(float));
    
    // Position Y (4 bytes, float)
    const uint8_t* py = reinterpret_cast<const uint8_t*>(&y);
    data.insert(data.end(), py, py + sizeof(float));
    
    return data;
}

MessageData MessageFactory::encodeMessageServer(std::string type, Entity entity, Entity entity_changes) const
{
    MessageData data;
    (void)type; (void)entity; (void)entity_changes;
    return data;
}
