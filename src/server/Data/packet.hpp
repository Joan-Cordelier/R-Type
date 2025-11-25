/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** packet
*/

/*
*/

#ifndef PACKET_HPP_
#define PACKET_HPP_

#include <vector>
#include <cstdint>
#include <array>
#include <string>
#include "LinearBuffer.hpp"

using PacketData = std::vector<uint8_t>;

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

constexpr std::array<Priority, 256> PriorityTable = [] {
    std::array<Priority, 256> table{};

    for (size_t i = 0; i < 256; ++i)
        table[i] = Priority::MEDIUM;

    table[INCOMPLETE] = Priority::ERROR; // Should not happen in queue
    table[PARSING_ERROR] = Priority::ERROR;
    table[DEATH] = Priority::CRITICAL;
    table[SHOOT] = Priority::HIGH;
    table[MOVE]  = Priority::LOW;

    return table;
}();

constexpr int VARIABLE_LEN = -1;

constexpr std::array<int, 256> PacketSizes = [] {
    std::array<int, 256> table{};

    for (size_t i = 0; i < 256; ++i)
        table[i] = 0;

    table[INCOMPLETE] = 0;
    table[PARSING_ERROR] = 0;
    table[DEATH] = 0;
    table[SHOOT] = 0;
    table[MOVE]  = 8;

    return table;
}();

struct Packet {
    OpCode opCode;
    uint8_t len;
    PacketData data;
};

Packet readPacketTCP(int fd);
Packet parsePacket(const std::vector<uint8_t>& rawData);
Packet extractPacketFromLinearBuffer(LinearBuffer& buffer);

#endif /* !PACKET_HPP_ */
