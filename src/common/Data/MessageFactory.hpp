/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** MessageFactory
*/

#ifndef MESSAGEFACTORY_HPP_
#define MESSAGEFACTORY_HPP_

#include <array>
#include <cstdint>
#include <netinet/in.h>
#include <string>
#include <vector>

#include "../ecs/entity_manager.hpp"
#include "EntityType.hpp"

enum Priority { CRITICAL, HIGH, MEDIUM, LOW, ERROR };

enum OpCode : uint8_t {
    INCOMPLETE = 0x00,
    PARSING_ERROR = 0x01,
    DEATH = 0x02,
    SHOOT = 0x04,
    CONNECT = 0x05,
    CONNECT_ACK = 0x06,
    START = 0x07,
    JOIN = 0x08,
    CRASH = 0x09,
    PLAYER = 0x0A,
    LINK = 0x0B,
    ENEMY = 0x0C,
    MOVE_SYNC = 0x0D,
    MOVE_INPUT = 0x0E,

    // Room management
    ROOM_LIST = 0x20,   // Server → Client: List of rooms
    ROOM_CREATE = 0x21, // Client → Server: Create new room
    ROOM_JOIN = 0x22,   // Client → Server: Join room by ID
    ROOM_LEAVE = 0x23,  // Client → Server: Leave current room
    ROOM_READY = 0x24, // Client → Room: Player connected to room, ready to play
};

using MessageData = std::vector<uint8_t>;

struct DecodedMessage {
    OpCode opCode;
    uint8_t len;
    Priority priority;
    MessageData data;
    uint32_t playerId = 0; // Set by server when receiving
    int tcpFd = -1;        // TCP file descriptor (for player lookup)
    sockaddr_in udpAddr{}; // UDP address (for player lookup)
};

struct PreparedMessage {
    MessageData data;
    Priority priority;
};

class MessageFactory {
public:
    static constexpr int VARIABLE_LEN = -1;

    struct Message {
        int len;
        Priority priority;
    };

    static MessageFactory &getInstance() {
        static MessageFactory instance;
        return instance;
    }

    MessageFactory(const MessageFactory &) = delete;
    MessageFactory &operator=(const MessageFactory &) = delete;

    DecodedMessage decode(const std::vector<uint8_t> &rawData);

    const Message &getMessageInfo(uint8_t opCode) const {
        return _messageTable[opCode];
    }

    int getMessageLength(uint8_t opCode) const {
        return _messageTable[opCode].len;
    }

    Priority getMessagePriority(uint8_t opCode) const {
        return _messageTable[opCode].priority;
    }

    bool decodeHeader(uint8_t op, size_t bufferSize, uint8_t secondByte,
                      size_t &headerSize, uint8_t &payloadLen,
                      Priority &priority) const;

    template <typename LinearBufferT>
    DecodedMessage decodeFromBuffer(LinearBufferT &buffer);

    PreparedMessage createMessage(OpCode opCode,
                                  const MessageData &payload = {}) const;

    MessageData encodeMessagePlayer(Entity entity) const;
    MessageData encodeMessageMovementPlayer(Entity entity, float x,
                                            float y) const;
    MessageData encodePlayerInfo(uint32_t playerId, Entity entity, float x,
                                 float y) const;
    MessageData encodeMessageServer(std::string type, Entity entity,
                                    Entity entity_changes) const;
    MessageData encodeMessageMove(EntityType type, Entity entity, float x,
                                  float y) const;
    MessageData encodeMessageDeath(EntityType type, Entity entity) const;
    MessageData encodeMessageEnemy(Entity entity, float x, float y) const;
    MessageData encodeMessageProjectile(Entity projectileEntity,
                                        Entity parentEntity,
                                        const std::string &ownerType) const;
    MessageData encodeMessageMoveInput(Entity entity, float vx, float vy) const;

private:
    MessageFactory();
    ~MessageFactory() = default;

    std::array<Message, 256> _messageTable;
    std::array<Message, 256> initMessageTable();
};

#endif
