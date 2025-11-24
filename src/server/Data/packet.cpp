/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** packet
*/

#include "packet.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

Packet readPacketTCP(int fd)
{
    Packet packet;
    uint8_t opcodeVal;

    ssize_t n = recv(fd, &opcodeVal, 1, 0);
    if (n <= 0) {
        packet.opCode = PARSING_ERROR;
        return packet;
    }
    packet.opCode = static_cast<OpCode>(opcodeVal);

    int expectedLen = PacketSizes[packet.opCode];
    
    if (expectedLen == VARIABLE_LEN) {
        if (recv(fd, &packet.len, 1, 0) <= 0) {
            packet.opCode = PARSING_ERROR;
            return packet;
        }
    } else
        packet.len = static_cast<uint8_t>(expectedLen);

    if (packet.len > 0) {
        packet.data.resize(packet.len);
        int received = 0;
        while (received < packet.len) {
            int ret = recv(fd, packet.data.data() + received, packet.len - received, 0);
            if (ret <= 0) {
                packet.opCode = PARSING_ERROR;
                return packet;
            }
            received += ret;
        }
    }

    return packet;
}

Packet parsePacket(const std::vector<uint8_t>& rawData)
{
    Packet packet;
    if (rawData.empty()) {
        packet.opCode = PARSING_ERROR;
        return packet;
    }

    packet.opCode = static_cast<OpCode>(rawData[0]);

    int expectedLen = PacketSizes[packet.opCode];
    size_t headerSize = 1;

    if (expectedLen == VARIABLE_LEN) {
        if (rawData.size() < 2) {
            packet.opCode = PARSING_ERROR;
            return packet;
        }
        packet.len = rawData[1];
        headerSize = 2;
    } else
        packet.len = static_cast<uint8_t>(expectedLen);
    if (rawData.size() < headerSize + packet.len) {
        packet.opCode = PARSING_ERROR;
        return packet;
    }

    if (packet.len > 0)
        packet.data.assign(rawData.begin() + headerSize, rawData.begin() + headerSize + packet.len);
    return packet;
}

Packet extractPacketFromBuffer(std::vector<uint8_t>& buffer)
{
    Packet packet;
    packet.opCode = INCOMPLETE;

    if (buffer.empty())
        return packet;

    uint8_t op = buffer[0];
    int expectedLen = PacketSizes[op];
    size_t headerSize = 1;
    uint8_t payloadLen = 0;

    if (expectedLen == VARIABLE_LEN) {
        if (buffer.size() < 2) return packet;
        payloadLen = buffer[1];
        headerSize = 2;
    } else
        payloadLen = static_cast<uint8_t>(expectedLen);
    if (buffer.size() < headerSize + payloadLen)
        return packet;
    packet.opCode = static_cast<OpCode>(op);
    packet.len = payloadLen;
    if (payloadLen > 0)
        packet.data.assign(buffer.begin() + headerSize, buffer.begin() + headerSize + payloadLen);
    buffer.erase(buffer.begin(), buffer.begin() + headerSize + payloadLen);
    return packet;
}

Packet extractPacketFromLinearBuffer(LinearBuffer& buffer)
{
    Packet packet;
    packet.opCode = INCOMPLETE;

    if (buffer.size() == 0)
        return packet;

    uint8_t op = buffer.peek(0);
    int expectedLen = PacketSizes[op];
    size_t headerSize = 1;
    uint8_t payloadLen = 0;

    if (expectedLen == VARIABLE_LEN) {
        if (buffer.size() < 2) return packet;
        payloadLen = buffer.peek(1);
        headerSize = 2;
    } else
        payloadLen = static_cast<uint8_t>(expectedLen);
    if (buffer.size() < headerSize + payloadLen)
        return packet;
    packet.opCode = static_cast<OpCode>(op);
    packet.len = payloadLen;
    
    buffer.consume(headerSize);
    
    if (payloadLen > 0)
        buffer.read(packet.data, payloadLen);
    return packet;
}

