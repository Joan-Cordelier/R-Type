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
#include <utility>
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
    UPGRADE_OPTIONS = 0x0F,
    UPGRADE_SELECT = 0x10,
    UPDATE_WEAPON = 0x11,
    COMPANION = 0x12,
    UPDATE_STATS = 0x13,
    CREATE_ROOM = 0x14,
    JOIN_ROOM = 0x15,
    LIST_ROOMS = 0x16,
    ROOM_LIST = 0x17,
    ROOM_CREATED = 0x18,
    JOIN_ACK = 0x19,
    DISCONNECT = 0x1A,
};

using MessageData = std::vector<uint8_t>;

struct DecodedMessage {
    OpCode opCode;
    uint32_t len;
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

    std::string getOpCodeName(OpCode opCode) const;
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

    bool decodeHeader(uint8_t op, size_t bufferSize, uint16_t lengthBytes, size_t &headerSize,
                      uint32_t &payloadLen, Priority &priority) const;

    MessageData compress(const MessageData &data) const;
    MessageData decompress(const MessageData &data, uint32_t originalSize) const;

    template <typename LinearBufferT> DecodedMessage decodeFromBuffer(LinearBufferT &buffer);

    PreparedMessage createMessage(OpCode opCode, const MessageData &payload = {}) const;

    MessageData encodeMessagePlayer(Entity entity) const;
    MessageData encodeMessageMovementPlayer(Entity entity, float x, float y) const;
    MessageData encodePlayerInfo(uint32_t playerId, Entity entity, float x, float y) const;
    MessageData encodeMessageServer(std::string type, Entity entity, Entity entity_changes) const;
    MessageData encodeMessageEnemy(Entity entity, float x, float y, const std::string &type) const;
    MessageData encodeMessageMove(EntityType type, Entity entity, float x, float y) const;

    MessageData encodeMessageDeath(EntityType type, Entity entity) const;
    MessageData encodeMessageProjectile(Entity projectileEntity, Entity parentEntity,
                                        const std::string &ownerType, float x, float y,
                                        float scale) const;
    MessageData encodeMessageUpdateWeapon(Entity entity, int damage, int nbBullets,
                                          float fireRate) const;
    MessageData encodeMessageMoveInput(Entity entity, float vx, float vy) const;
    MessageData encodeMessageUpgradeOptions(const std::vector<std::string> &upgradeIds) const;
    MessageData encodeMessageUpgradeSelect(uint8_t index) const;
    MessageData encodeMessageCompanion(Entity entity, float x, float y, uint8_t type) const;
    MessageData encodeMessageUpdateStats(Entity entity, int hp, int maxHp, int speed) const;

    MessageData encodeMessageRoomList(const std::vector<std::pair<uint32_t, uint8_t>> &rooms) const;
    MessageData encodeMessageRoomCreated(uint32_t roomId) const;
    MessageData encodeMessageJoinAck(uint32_t roomId, bool success) const;

private:
    MessageFactory();
    ~MessageFactory() = default;

    std::array<Message, 256> _messageTable;
    std::array<Message, 256> initMessageTable();
};

#endif
