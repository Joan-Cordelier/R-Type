/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** MessageFactory
*/

#ifndef MESSAGEFACTORY_HPP_
#define MESSAGEFACTORY_HPP_

#include <vector>
#include <cstdint>
#include <array>

enum Priority {
    CRITICAL,
    HIGH,
    MEDIUM,
    LOW,
    ERROR
};

enum OpCode : uint8_t {
    INCOMPLETE = 0x00,
    PARSING_ERROR = 0x01,
    DEATH = 0x02,
    MOVE = 0x03,
    SHOOT = 0x04
};

using MessageData = std::vector<uint8_t>;

struct DecodedMessage {
    OpCode opCode;
    uint8_t len;
    Priority priority;
    MessageData data;
};

class MessageFactory {
    public:
        static constexpr int VARIABLE_LEN = -1;
        
        struct Message {
            int len;
            Priority priority;
        };
        
        static MessageFactory& getInstance() {
            static MessageFactory instance;
            return instance;
        }
        
        MessageFactory(const MessageFactory&) = delete;
        MessageFactory& operator=(const MessageFactory&) = delete;
        
        DecodedMessage decode(const std::vector<uint8_t>& rawData);
        
        const Message& getMessageInfo(uint8_t opCode) const {
            return _messageTable[opCode];
        }
        
        int getMessageLength(uint8_t opCode) const {
            return _messageTable[opCode].len;
        }
        
        Priority getMessagePriority(uint8_t opCode) const {
            return _messageTable[opCode].priority;
        }
        
        bool decodeHeader(uint8_t op, size_t bufferSize, 
                         uint8_t secondByte, size_t& headerSize, 
                         uint8_t& payloadLen, Priority& priority) const;
        
        template<typename LinearBufferT>
        DecodedMessage decodeFromBuffer(LinearBufferT& buffer);
        
    private:
        MessageFactory();
        ~MessageFactory() = default;
        
        std::array<Message, 256> _messageTable;
        std::array<Message, 256> initMessageTable();
};

#endif
