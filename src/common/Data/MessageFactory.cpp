/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** MessageFactory
*/

#include "MessageFactory.hpp"
#include "LinearBuffer.hpp"
#include <cstring>
#include <zlib.h>

MessageFactory::MessageFactory() : _messageTable(initMessageTable()) {
}

std::array<MessageFactory::Message, 256> MessageFactory::initMessageTable() {
    std::array<Message, 256> table{};

    for (size_t i = 0; i < 256; ++i)
        table[i] = {0, Priority::ERROR};

    table[INCOMPLETE] = {0, Priority::ERROR};
    table[PARSING_ERROR] = {0, Priority::ERROR};
    table[DEATH] = {5, Priority::CRITICAL};
    table[SHOOT] = {38, Priority::HIGH};
    table[MOVE_SYNC] = {13, Priority::LOW};
    table[MOVE_INPUT] = {12, Priority::LOW};
    table[CONNECT] = {0, Priority::CRITICAL};
    table[CONNECT_ACK] = {4, Priority::CRITICAL};
    table[START] = {0, Priority::CRITICAL};
    table[JOIN] = {1, Priority::CRITICAL};
    table[CRASH] = {1, Priority::CRITICAL};
    table[PLAYER] = {17, Priority::CRITICAL};  // 4 playerId + 4 entity + 4 x + 4 y + 1 skinIndex
    table[LINK] = {4, Priority::CRITICAL};
    table[ENEMY] = {28, Priority::HIGH};
    table[UPGRADE_OPTIONS] = {VARIABLE_LEN, Priority::MEDIUM};
    table[UPGRADE_SELECT] = {1, Priority::MEDIUM};
    table[UPDATE_WEAPON] = {16, Priority::HIGH};
    table[COMPANION] = {13, Priority::HIGH};
    table[UPDATE_STATS] = {16, Priority::HIGH};
    table[CREATE_ROOM] = {3, Priority::MEDIUM};  // maxPlayers (1) + gameMode (1) + difficulty (1)
    table[JOIN_ROOM] = {4, Priority::MEDIUM};   // RoomID (4 bytes)
    table[LIST_ROOMS] = {0, Priority::LOW};     // No payload
    table[ROOM_LIST] = {VARIABLE_LEN, Priority::LOW};
    table[ROOM_CREATED] = {4, Priority::MEDIUM}; // RoomID
    table[JOIN_ACK] = {5, Priority::MEDIUM};     // RoomID (4) + Success (1)
    table[DISCONNECT] = {0, Priority::CRITICAL};
    table[REGISTER] = {VARIABLE_LEN, Priority::MEDIUM};     // username + password
    table[REGISTER_ACK] = {VARIABLE_LEN, Priority::MEDIUM}; // success + userId + errorMsg
    table[LOGIN] = {VARIABLE_LEN, Priority::MEDIUM};        // username + password
    table[LOGIN_ACK] = {VARIABLE_LEN, Priority::MEDIUM};    // success + userId + username + errorMsg
    table[GUEST_LOGIN] = {0, Priority::MEDIUM};             // No payload
    table[GUEST_LOGIN_ACK] = {VARIABLE_LEN, Priority::MEDIUM}; // guestId + guestName
    table[CHAT_MESSAGE] = {VARIABLE_LEN, Priority::MEDIUM};    // message text
    table[CHAT_BROADCAST] = {VARIABLE_LEN, Priority::MEDIUM};  // senderId + senderName + message

    return table;
}

DecodedMessage MessageFactory::decode(const std::vector<uint8_t> &rawData) {
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

bool MessageFactory::decodeHeader(uint8_t op, size_t bufferSize, uint16_t lengthBytes,
                                  size_t &headerSize, uint32_t &payloadLen,
                                  Priority &priority) const {
    int expectedLen = _messageTable[op].len;
    priority = _messageTable[op].priority;

    headerSize = 1;

    if (expectedLen == VARIABLE_LEN) {
        if (bufferSize < 3)
            return false;

        // bool isCompressed = (lengthBytes & 0x8000) != 0; // Unused here, handled in caller
        payloadLen = lengthBytes & 0x7FFF;
        headerSize = 3; // Op + 2 bytes len
    } else {
        payloadLen = static_cast<uint32_t>(expectedLen);
    }

    if (bufferSize < headerSize + payloadLen)
        return false;

    return true;
}

template <typename LinearBufferT>
DecodedMessage MessageFactory::decodeFromBuffer(LinearBufferT &buffer) {
    DecodedMessage message;
    message.opCode = INCOMPLETE;

    if (buffer.size() == 0)
        return message;

    uint8_t op = buffer.peek(0);
    int expectedLen = getMessageLength(op);
    uint16_t lengthBytes = 0;

    if (expectedLen == VARIABLE_LEN) {
        if (buffer.size() >= 3) {
            uint8_t b1 = buffer.peek(1);
            uint8_t b2 = buffer.peek(2);
            lengthBytes = (static_cast<uint16_t>(b1) << 8) | b2;
        }
    }

    size_t headerSize = 0;
    uint32_t payloadLen = 0;

    if (!decodeHeader(op, buffer.size(), lengthBytes, headerSize, payloadLen, message.priority))
        return message;

    message.opCode = static_cast<OpCode>(op);
    message.len = payloadLen;

    buffer.consume(headerSize);

    if (payloadLen > 0) {
        message.data.resize(payloadLen);
        buffer.read(message.data, payloadLen);

        // Decompress if flag set
        if (expectedLen == VARIABLE_LEN && (lengthBytes & 0x8000)) {
            // Use 1MB max buffer for safety, usually enough for game state
            MessageData decompressed = decompress(message.data, 1024 * 1024);
            if (decompressed.empty() && !message.data.empty()) {
                message.opCode = PARSING_ERROR;
                message.priority = Priority::ERROR;
            } else {
                message.data = decompressed;
                message.len = static_cast<uint32_t>(message.data.size());
            }
        }
    }

    return message;
}

template DecodedMessage MessageFactory::decodeFromBuffer<LinearBuffer>(LinearBuffer &buffer);

PreparedMessage MessageFactory::createMessage(OpCode opCode, const MessageData &payload) const {
    PreparedMessage prepared;

    int expectedLen = _messageTable[opCode].len;

    prepared.data.push_back(static_cast<uint8_t>(opCode));

    if (expectedLen == VARIABLE_LEN) {
        bool doCompress = payload.size() > 1024;
        MessageData finalPayload;

        if (doCompress) {
            finalPayload = compress(payload);
            if (finalPayload.empty()) {
                doCompress = false;
                finalPayload = payload;
            }
        } else {
            finalPayload = payload;
        }

        uint16_t lenVal = static_cast<uint16_t>(finalPayload.size());
        if (doCompress) {
            lenVal |= 0x8000;
        }

        prepared.data.push_back(static_cast<uint8_t>((lenVal >> 8) & 0xFF));
        prepared.data.push_back(static_cast<uint8_t>(lenVal & 0xFF));

        if (!finalPayload.empty()) {
            prepared.data.insert(prepared.data.end(), finalPayload.begin(), finalPayload.end());
        }
    } else {
        if (!payload.empty()) {
            prepared.data.insert(prepared.data.end(), payload.begin(), payload.end());
        }
    }

    prepared.priority = _messageTable[opCode].priority;

    return prepared;
}

MessageData MessageFactory::encodeMessagePlayer(Entity entity) const {
    MessageData data;
    data.push_back(static_cast<uint8_t>((entity >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(entity & 0xFF));
    return data;
}

MessageData MessageFactory::encodeMessageProjectile(Entity projectileEntity, Entity parentEntity,
                                                    const std::string &ownerType, float x, float y,
                                                    float vx, float vy, float scale) const {
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

    // Encode vx coordinate (4 bytes)
    uint32_t vxInt;
    std::memcpy(&vxInt, &vx, sizeof(float));
    data.push_back(static_cast<uint8_t>((vxInt >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((vxInt >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((vxInt >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(vxInt & 0xFF));

    // Encode vy coordinate (4 bytes)
    uint32_t vyInt;
    std::memcpy(&vyInt, &vy, sizeof(float));
    data.push_back(static_cast<uint8_t>((vyInt >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((vyInt >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((vyInt >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(vyInt & 0xFF));

    // Encode scale (4 bytes)
    uint32_t scaleInt;
    std::memcpy(&scaleInt, &scale, sizeof(float));
    data.push_back(static_cast<uint8_t>((scaleInt >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((scaleInt >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((scaleInt >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(scaleInt & 0xFF));

    return data;
}

MessageData MessageFactory::encodeMessageMove(EntityType type, Entity entity, float x,
                                              float y) const {
    MessageData data;

    data.push_back(static_cast<uint8_t>(type));

    data.push_back(static_cast<uint8_t>((entity >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(entity & 0xFF));

    const uint8_t *px = reinterpret_cast<const uint8_t *>(&x);
    data.insert(data.end(), px, px + sizeof(float));

    const uint8_t *py = reinterpret_cast<const uint8_t *>(&y);
    data.insert(data.end(), py, py + sizeof(float));

    return data;
}

MessageData MessageFactory::encodeMessageDeath(EntityType type, Entity entity) const {
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

MessageData MessageFactory::encodeMessageMoveInput(Entity entity, float vx, float vy) const {
    MessageData data;

    data.push_back(static_cast<uint8_t>((entity >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(entity & 0xFF));

    const uint8_t *pvx = reinterpret_cast<const uint8_t *>(&vx);
    data.insert(data.end(), pvx, pvx + sizeof(float));

    const uint8_t *pvy = reinterpret_cast<const uint8_t *>(&vy);
    data.insert(data.end(), pvy, pvy + sizeof(float));

    return data;
}

MessageData MessageFactory::encodeMessageEnemy(Entity entity, float x, float y,
                                               const std::string &type) const {
    MessageData data;

    data.push_back(static_cast<uint8_t>((entity >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(entity & 0xFF));

    const uint8_t *px = reinterpret_cast<const uint8_t *>(&x);
    data.insert(data.end(), px, px + sizeof(float));

    const uint8_t *py = reinterpret_cast<const uint8_t *>(&y);
    data.insert(data.end(), py, py + sizeof(float));

    std::string fixedType = type;
    fixedType.resize(16, '\0');
    for (char c : fixedType) {
        data.push_back(static_cast<uint8_t>(c));
    }

    return data;
}

MessageData MessageFactory::encodePlayerInfo(uint32_t playerId, Entity entity, float x,
                                             float y, uint8_t skinIndex) const {
    MessageData data;

    data.push_back(static_cast<uint8_t>((playerId >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((playerId >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((playerId >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(playerId & 0xFF));

    data.push_back(static_cast<uint8_t>((entity >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(entity & 0xFF));

    const uint8_t *px = reinterpret_cast<const uint8_t *>(&x);
    data.insert(data.end(), px, px + sizeof(float));

    const uint8_t *py = reinterpret_cast<const uint8_t *>(&y);
    data.insert(data.end(), py, py + sizeof(float));

    data.push_back(skinIndex);

    return data;
}

MessageData MessageFactory::encodeMessageServer(std::string type, Entity entity,
                                                Entity entity_changes) const {
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

MessageData MessageFactory::encodeMessageMovementPlayer(Entity entity, float x, float y) const {
    MessageData data;

    data.push_back(static_cast<uint8_t>((entity >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(entity & 0xFF));

    const uint8_t *px = reinterpret_cast<const uint8_t *>(&x);
    data.insert(data.end(), px, px + sizeof(float));

    const uint8_t *py = reinterpret_cast<const uint8_t *>(&y);
    data.insert(data.end(), py, py + sizeof(float));

    return data;
}

MessageData
MessageFactory::encodeMessageUpgradeOptions(const std::vector<std::string> &upgradeIds) const {
    MessageData data;
    // Format: [Count] [Len1][Str1] [Len2][Str2] ...

    data.push_back(static_cast<uint8_t>(upgradeIds.size()));

    for (const auto &id : upgradeIds) {
        // Truncate to 255 if insanely long (shouldn't happen for IDs)
        uint8_t len = static_cast<uint8_t>(std::min(id.size(), static_cast<size_t>(255)));
        data.push_back(len);

        const uint8_t *ptr = reinterpret_cast<const uint8_t *>(id.c_str());
        data.insert(data.end(), ptr, ptr + len);
    }

    return data;
}

MessageData MessageFactory::encodeMessageUpgradeSelect(uint8_t index) const {
    MessageData data;
    data.push_back(index);
    return data;
}

MessageData MessageFactory::encodeMessageUpdateWeapon(Entity entity, int damage, int nbBullets,
                                                      float fireRate) const {
    MessageData data;

    // Entity
    data.push_back(static_cast<uint8_t>((entity >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(entity & 0xFF));

    // Damage
    data.push_back(static_cast<uint8_t>((damage >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((damage >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((damage >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(damage & 0xFF));

    // NbBullets
    data.push_back(static_cast<uint8_t>((nbBullets >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((nbBullets >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((nbBullets >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(nbBullets & 0xFF));

    // FireRate
    const uint8_t *pfr = reinterpret_cast<const uint8_t *>(&fireRate);
    data.insert(data.end(), pfr, pfr + sizeof(float));

    return data;
}

MessageData MessageFactory::encodeMessageCompanion(Entity entity, float x, float y,
                                                   uint8_t type) const {
    MessageData data;

    // Entity ID (4 bytes)
    data.push_back(static_cast<uint8_t>((entity >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(entity & 0xFF));

    // X (4 bytes)
    const uint8_t *xBytes = reinterpret_cast<const uint8_t *>(&x);
    for (size_t i = 0; i < sizeof(float); ++i)
        data.push_back(xBytes[i]);

    // Y (4 bytes)
    const uint8_t *yBytes = reinterpret_cast<const uint8_t *>(&y);
    for (size_t i = 0; i < sizeof(float); ++i)
        data.push_back(yBytes[i]);

    // Type (1 byte)
    data.push_back(type);

    return data;
}

MessageData MessageFactory::encodeMessageUpdateStats(Entity entity, int hp, int maxHp,
                                                     int speed) const {
    MessageData data;

    // Entity ID (4 bytes)
    data.push_back(static_cast<uint8_t>((entity >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((entity >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(entity & 0xFF));

    // HP (4 bytes)
    data.push_back(static_cast<uint8_t>((hp >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((hp >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((hp >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(hp & 0xFF));

    // MaxHP (4 bytes)
    data.push_back(static_cast<uint8_t>((maxHp >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((maxHp >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((maxHp >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(maxHp & 0xFF));

    // Speed (4 bytes)
    data.push_back(static_cast<uint8_t>((speed >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((speed >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((speed >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(speed & 0xFF));

    return data;
}

MessageData MessageFactory::encodeMessageRoomList(
    const std::vector<std::tuple<uint32_t, uint8_t, uint8_t>> &rooms) const {
    MessageData data;

    data.push_back(static_cast<uint8_t>(rooms.size()));

    for (const auto &room : rooms) {
        uint32_t roomId = std::get<0>(room);
        uint8_t playerCount = std::get<1>(room);
        uint8_t maxPlayers = std::get<2>(room);

        data.push_back(static_cast<uint8_t>((roomId >> 24) & 0xFF));
        data.push_back(static_cast<uint8_t>((roomId >> 16) & 0xFF));
        data.push_back(static_cast<uint8_t>((roomId >> 8) & 0xFF));
        data.push_back(static_cast<uint8_t>(roomId & 0xFF));

        data.push_back(playerCount);
        data.push_back(maxPlayers);
    }

    return data;
}

MessageData MessageFactory::encodeMessageRoomCreated(uint32_t roomId) const {
    MessageData data;

    data.push_back(static_cast<uint8_t>((roomId >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((roomId >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((roomId >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(roomId & 0xFF));

    return data;
}

MessageData MessageFactory::encodeMessageJoinAck(uint32_t roomId, bool success) const {
    MessageData data;

    data.push_back(static_cast<uint8_t>((roomId >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((roomId >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((roomId >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(roomId & 0xFF));

    data.push_back(success ? 1 : 0);

    return data;
}

MessageData MessageFactory::encodeMessageCreateRoom(uint8_t maxPlayers, uint8_t gameMode,
                                                    uint8_t difficulty) const {
    MessageData data;
    data.push_back(maxPlayers);
    data.push_back(gameMode);
    data.push_back(difficulty);
    return data;
}

MessageData MessageFactory::compress(const MessageData &data) const {
    uLong sourceLen = data.size();
    uLong destLen = compressBound(sourceLen);
    MessageData dest(destLen);

    int res = ::compress(dest.data(), &destLen, data.data(), sourceLen);
    if (res != Z_OK) {
        return {};
    }
    dest.resize(destLen);
    return dest;
}

MessageData MessageFactory::decompress(const MessageData &data,
                                       uint32_t maxUncompressedSize) const {
    MessageData dest(maxUncompressedSize);
    uLong destLen = maxUncompressedSize;
    uLong sourceLen = data.size();

    int res = ::uncompress(dest.data(), &destLen, data.data(), sourceLen);
    if (res != Z_OK) {
        return {};
    }
    dest.resize(destLen);
    return dest;
}

std::string MessageFactory::getOpCodeName(OpCode opCode) const {
    switch (opCode) {
        case INCOMPLETE:      return "INCOMPLETE";
        case PARSING_ERROR:   return "PARSING_ERROR";
        case CRASH:           return "CRASH";
        case CONNECT:         return "CONNECT";
        case CONNECT_ACK:     return "CONNECT_ACK";
        case DISCONNECT:      return "DISCONNECT";
        case LINK:            return "LINK";
        case CREATE_ROOM:     return "CREATE_ROOM";
        case JOIN_ROOM:       return "JOIN_ROOM";
        case JOIN_ACK:        return "JOIN_ACK";
        case LIST_ROOMS:      return "LIST_ROOMS";
        case ROOM_LIST:       return "ROOM_LIST";
        case ROOM_CREATED:    return "ROOM_CREATED";
        case START:           return "START";
        case JOIN:            return "JOIN";
        case PLAYER:          return "PLAYER";
        case ENEMY:           return "ENEMY";
        case DEATH:           return "DEATH";
        case SHOOT:           return "SHOOT";
        case MOVE_SYNC:       return "MOVE_SYNC";
        case MOVE_INPUT:      return "MOVE_INPUT";
        case COMPANION:       return "COMPANION";
        case UPDATE_WEAPON:   return "UPDATE_WEAPON";
        case UPDATE_STATS:    return "UPDATE_STATS";
        case UPGRADE_OPTIONS: return "UPGRADE_OPTIONS";
        case UPGRADE_SELECT:  return "UPGRADE_SELECT";
        case REGISTER:        return "REGISTER";
        case REGISTER_ACK:    return "REGISTER_ACK";
        case LOGIN:           return "LOGIN";
        case LOGIN_ACK:       return "LOGIN_ACK";
        case GUEST_LOGIN:     return "GUEST_LOGIN";
        case GUEST_LOGIN_ACK: return "GUEST_LOGIN_ACK";
        default:              return "UNKNOWN_" + std::to_string(static_cast<int>(opCode));
    }
}

// ==================== Authentication Messages ====================

MessageData MessageFactory::encodeMessageRegister(const std::string& username, const std::string& password) const {
    MessageData data;
    
    // Username length (1 byte) + username + password length (1 byte) + password
    data.push_back(static_cast<uint8_t>(username.size()));
    for (char c : username) data.push_back(static_cast<uint8_t>(c));
    
    data.push_back(static_cast<uint8_t>(password.size()));
    for (char c : password) data.push_back(static_cast<uint8_t>(c));
    
    return data;
}

MessageData MessageFactory::encodeMessageRegisterAck(bool success, uint32_t userId, const std::string& errorMsg) const {
    MessageData data;
    
    data.push_back(success ? 1 : 0);
    
    // User ID (4 bytes)
    data.push_back(static_cast<uint8_t>((userId >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((userId >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((userId >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(userId & 0xFF));
    
    // Error message (if any)
    data.push_back(static_cast<uint8_t>(errorMsg.size()));
    for (char c : errorMsg) data.push_back(static_cast<uint8_t>(c));
    
    return data;
}

MessageData MessageFactory::encodeMessageLogin(const std::string& username, const std::string& password) const {
    // Same format as register
    return encodeMessageRegister(username, password);
}

MessageData MessageFactory::encodeMessageLoginAck(bool success, uint32_t userId, const std::string& username, const std::string& errorMsg) const {
    MessageData data;
    
    data.push_back(success ? 1 : 0);
    
    // User ID (4 bytes)
    data.push_back(static_cast<uint8_t>((userId >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((userId >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((userId >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(userId & 0xFF));
    
    // Username
    data.push_back(static_cast<uint8_t>(username.size()));
    for (char c : username) data.push_back(static_cast<uint8_t>(c));
    
    // Error message
    data.push_back(static_cast<uint8_t>(errorMsg.size()));
    for (char c : errorMsg) data.push_back(static_cast<uint8_t>(c));
    
    return data;
}

MessageData MessageFactory::encodeMessageGuestLoginAck(uint32_t guestId, const std::string& guestName) const {
    MessageData data;
    
    // Guest ID (4 bytes)
    data.push_back(static_cast<uint8_t>((guestId >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((guestId >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((guestId >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(guestId & 0xFF));
    
    // Guest name
    data.push_back(static_cast<uint8_t>(guestName.size()));
    for (char c : guestName) data.push_back(static_cast<uint8_t>(c));
    
    return data;
}

MessageData MessageFactory::encodeMessageChat(const std::string& message) const {
    MessageData data;
    
    // Message length (1 byte, max 255 chars)
    uint8_t msgLen = static_cast<uint8_t>(std::min(message.size(), static_cast<size_t>(255)));
    data.push_back(msgLen);

    // Message content
    for (size_t i = 0; i < msgLen; ++i) {
        data.push_back(static_cast<uint8_t>(message[i]));
    }

    return data;
}

MessageData MessageFactory::encodeMessageChatBroadcast(uint32_t senderId, const std::string& senderName, const std::string& message) const {
    MessageData data;

    // Sender ID (4 bytes)
    data.push_back(static_cast<uint8_t>((senderId >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((senderId >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((senderId >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(senderId & 0xFF));

    // Sender name length + name
    uint8_t nameLen = static_cast<uint8_t>(std::min(senderName.size(), static_cast<size_t>(32)));
    data.push_back(nameLen);
    for (size_t i = 0; i < nameLen; ++i) {
        data.push_back(static_cast<uint8_t>(senderName[i]));
    }
    
    // Message length + message
    uint8_t msgLen = static_cast<uint8_t>(std::min(message.size(), static_cast<size_t>(255)));
    data.push_back(msgLen);
    for (size_t i = 0; i < msgLen; ++i) {
        data.push_back(static_cast<uint8_t>(message[i]));
    }

    return data;
}
