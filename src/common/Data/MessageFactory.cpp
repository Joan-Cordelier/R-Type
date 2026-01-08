/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** MessageFactory
*/

#include "MessageFactory.hpp"
#include "LinearBuffer.hpp"
#include <cstring>

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
    table[DEATH] = {5, Priority::CRITICAL};
    table[SHOOT] = {26, Priority::HIGH};
    table[MOVE_SYNC] = {13, Priority::LOW};
    table[MOVE_INPUT] = {12, Priority::LOW};
    table[CONNECT] = {0, Priority::CRITICAL};
    table[CONNECT_ACK] = {4, Priority::CRITICAL};
    table[START] = {0, Priority::CRITICAL}; 
    table[JOIN] = {1, Priority::CRITICAL}; 
    table[CRASH] = {1, Priority::CRITICAL}; 
    table[PLAYER] = {16, Priority::CRITICAL};
    table[LINK] = {4, Priority::CRITICAL};
    table[ENEMY] = {12, Priority::HIGH};

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

MessageData MessageFactory::encodeMessageProjectile(Entity projectileEntity, Entity parentEntity, const std::string& ownerType, float x, float y) const
{
    MessageData data;
    
    data.push_back(static_cast<uint8_t>((projectileEntity >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((projectileEntity >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((projectileEntity >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(projectileEntity & 0xFF));

    data.push_back(static_cast<uint8_t>((parentEntity >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((parentEntity >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((parentEntity >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(parentEntity & 0xFF));

    std::string fixedOwnerType = ownerType;
    fixedOwnerType.resize(10, '\0');
    for (char c : fixedOwnerType) {
        data.push_back(static_cast<uint8_t>(c));
    }
    
    // Encode x coordinate (4 bytes)
    uint32_t xInt;
    std::memcpy(&xInt, &x, sizeof(float));
    data.push_back(static_cast<uint8_t>((xInt >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((xInt >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((xInt >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(xInt & 0xFF));
    
    // Encode y coordinate (4 bytes)
    uint32_t yInt;
    std::memcpy(&yInt, &y, sizeof(float));
    data.push_back(static_cast<uint8_t>((yInt >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((yInt >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((yInt >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(yInt & 0xFF));
    
    return data;
}

MessageData MessageFactory::encodeMessageMove(EntityType type, Entity entity, float x, float y) const
{
    MessageData data;
    
    data.push_back(static_cast<uint8_t>(type));
    
    data.push_back(static_cast<uint8_t>((entity >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(entity & 0xFF));
    
    const uint8_t* px = reinterpret_cast<const uint8_t*>(&x);
    data.insert(data.end(), px, px + sizeof(float));
    
    const uint8_t* py = reinterpret_cast<const uint8_t*>(&y);
    data.insert(data.end(), py, py + sizeof(float));
    
    return data;
}

MessageData MessageFactory::encodeMessageDeath(EntityType type, Entity entity) const
{
    MessageData data;
    
    // Entity Type (1 byte)
    data.push_back(static_cast<uint8_t>(type));
    
    // Entity ID (4 bytes, big-endian)
    data.push_back(static_cast<uint8_t>((entity >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(entity & 0xFF));
    
    return data;
}

MessageData MessageFactory::encodeMessageMoveInput(Entity entity, float vx, float vy) const
{
    MessageData data;
    
    data.push_back(static_cast<uint8_t>((entity >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(entity & 0xFF));
    
    const uint8_t* pvx = reinterpret_cast<const uint8_t*>(&vx);
    data.insert(data.end(), pvx, pvx + sizeof(float));
    
    const uint8_t* pvy = reinterpret_cast<const uint8_t*>(&vy);
    data.insert(data.end(), pvy, pvy + sizeof(float));
    
    return data;
}

MessageData MessageFactory::encodeMessageEnemy(Entity entity, float x, float y) const
{
    MessageData data;
    
    data.push_back(static_cast<uint8_t>((entity >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(entity & 0xFF));
    
    const uint8_t* px = reinterpret_cast<const uint8_t*>(&x);
    data.insert(data.end(), px, px + sizeof(float));
    
    const uint8_t* py = reinterpret_cast<const uint8_t*>(&y);
    data.insert(data.end(), py, py + sizeof(float));
    
    return data;
}

MessageData MessageFactory::encodePlayerInfo(uint32_t playerId, Entity entity, float x, float y) const
{
    MessageData data;
    
    data.push_back(static_cast<uint8_t>((playerId >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((playerId >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((playerId >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(playerId & 0xFF));
    
    data.push_back(static_cast<uint8_t>((entity >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(entity & 0xFF));
    
    const uint8_t* px = reinterpret_cast<const uint8_t*>(&x);
    data.insert(data.end(), px, px + sizeof(float));
    
    const uint8_t* py = reinterpret_cast<const uint8_t*>(&y);
    data.insert(data.end(), py, py + sizeof(float));
    
    return data;
}

MessageData MessageFactory::encodeMessageServer(std::string type, Entity entity, Entity entity_changes) const
{
    MessageData data;
    
    std::string fixedType = type;
    fixedType.resize(10, '\0');
    for (char c : fixedType) {
        data.push_back(static_cast<uint8_t>(c));
    }
    
    data.push_back(static_cast<uint8_t>((entity >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(entity & 0xFF));
    
    data.push_back(static_cast<uint8_t>((entity_changes >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity_changes >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity_changes >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(entity_changes & 0xFF));
    
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
    data.insert(data.end(), px, px + sizeof(float));
    
    const uint8_t* py = reinterpret_cast<const uint8_t*>(&y);
    data.insert(data.end(), py, py + sizeof(float));
    
    return data;
}
