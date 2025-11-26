/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** ThreadedQueue
*/

#include "ThreadedQueue.hpp"

ThreadedQueue::~ThreadedQueue()
{
    std::queue<MessageData> empty;
    std::queue<MessageData> emptyHigh;
    std::queue<MessageData> emptyMedium;
    std::queue<MessageData> emptyLow;

    std::swap(_critical, empty);
    std::swap(_high, emptyHigh);
    std::swap(_medium, emptyMedium);
    std::swap(_low, emptyLow);
}

MessageData ThreadedQueue::pop(Priority level)
{
    std::lock_guard<std::mutex> lock(_mutex);
    std::queue<MessageData>* q = nullptr;

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
        MessageData data = q->front();
        q->pop();
        return data;
    }
    return {PARSING_ERROR};
}

void ThreadedQueue::push(Priority level, const MessageData& data)
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
        case ERROR:
            // ERROR priority messages are not queued
            break;
    }
}

bool ThreadedQueue::isEmpty(Priority level) const
{
    std::lock_guard<std::mutex> lock(_mutex);
    switch (level) {
        case CRITICAL:
            return _critical.empty();
        case HIGH:
            return _high.empty();
        case MEDIUM:
            return _medium.empty();
        case LOW:
            return _low.empty();
        default:
            return true;
    }
}

bool ThreadedQueue::isEmpty() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _critical.empty() && _high.empty() && _medium.empty() && _low.empty();
}
