/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** Queue
*/

#include "Queue.hpp"

Queue::~Queue()
{
    std::queue<PacketData> empty;
    std::queue<PacketData> emptyHigh;
    std::queue<PacketData> emptyMedium;
    std::queue<PacketData> emptyLow;

    std::swap(_critical, empty);
    std::swap(_high, emptyHigh);
    std::swap(_medium, emptyMedium);
    std::swap(_low, emptyLow);
}

Queue::PacketData Queue::pop(Priority level)
{
    std::lock_guard<std::mutex> lock(_mutex);
    std::queue<PacketData>* q = nullptr;

    switch (level) {
        case CRITICAL:
            q = &_critical;
            break;
        case HIGH:
            q = &_high;
            break;
        case MEDIUM:
            q = &_medium;
            break;
        case LOW:
            q = &_low;
            break;
        default:
            return {PARSING_ERROR};
    }

    if (q && !q->empty()) {
        PacketData data = q->front();
        q->pop();
        return data;
    }
    return {PARSING_ERROR};
}

void Queue::push(Priority level, const PacketData& data)
{
    std::lock_guard<std::mutex> lock(_mutex);
    switch (level) {
        case CRITICAL:
            _critical.push(data);
            break;
        case HIGH:
            _high.push(data);
            break;
        case MEDIUM:
            _medium.push(data);
            break;
        case LOW:
            _low.push(data);
            break;
    }
}
