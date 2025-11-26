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
        
        static DecodedMessage decode(const std::vector<uint8_t>& rawData);
        
        static const Message& getMessageInfo(uint8_t opCode) {
            return MessageTable[opCode];
        }
        
        static int getMessageLength(uint8_t opCode) {
            return MessageTable[opCode].len;
        }
        
        static Priority getMessagePriority(uint8_t opCode) {
            return MessageTable[opCode].priority;
        }
        
        template<typename LinearBufferT>
        static DecodedMessage decodeFromBuffer(LinearBufferT& buffer) {
            DecodedMessage message;
            message.opCode = INCOMPLETE;
            
            if (buffer.size() == 0)
                return message;
            
            uint8_t op = buffer.peek(0);
            int expectedLen = MessageTable[op].len;
            message.priority = MessageTable[op].priority;
            
            size_t headerSize = 1;
            uint8_t payloadLen = 0;
            
            if (expectedLen == VARIABLE_LEN) {
                if (buffer.size() < 2) return message;
                payloadLen = buffer.peek(1);
                headerSize = 2;
            } else
                payloadLen = static_cast<uint8_t>(expectedLen);
            
            if (buffer.size() < headerSize + payloadLen)
                return message;
            
            message.opCode = static_cast<OpCode>(op);
            message.len = payloadLen;
            
            buffer.consume(headerSize);
            
            if (payloadLen > 0)
                buffer.read(message.data, payloadLen);
            
            return message;
        }
    protected:
    private:
        static constexpr std::array<Message, 256> MessageTable = [] {
            std::array<Message, 256> table{};

            for (size_t i = 0; i < 256; ++i)
                table[i] = {0, Priority::MEDIUM};

            table[INCOMPLETE] = {0, Priority::ERROR}; // Should not happen in queue
            table[PARSING_ERROR] = {0, Priority::ERROR};
            table[DEATH] = {0, Priority::CRITICAL};
            table[SHOOT] = {0, Priority::HIGH};
            table[MOVE] = {8, Priority::LOW};

            return table;
        }();
};

#endif /* !MESSAGEFACTORY_HPP_ */
