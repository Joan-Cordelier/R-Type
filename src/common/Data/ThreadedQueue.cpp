/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** ThreadedQueue
*/

#include "ThreadedQueue.hpp"
#include "MessageFactory.hpp"

template<typename T>
ThreadedQueue<T>::~ThreadedQueue()
{
    std::queue<T> empty;
    std::queue<T> emptyHigh;
    std::queue<T> emptyMedium;
    std::queue<T> emptyLow;

    std::swap(_critical, empty);
    std::swap(_high, emptyHigh);
    std::swap(_medium, emptyMedium);
    std::swap(_low, emptyLow);
}

template<typename T>
std::optional<T> ThreadedQueue<T>::pop(Priority level)
{
    std::lock_guard<std::mutex> lock(_mutex);
    std::queue<T>* q = nullptr;

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
            return std::nullopt;
    }

    if (q && !q->empty()) {
        T data = q->front();
        q->pop();
        return data;
    }
    return std::nullopt;
}

template<typename T>
void ThreadedQueue<T>::push(Priority level, const T& data)
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

template<typename T>
bool ThreadedQueue<T>::isEmpty(Priority level) const
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

template<typename T>
bool ThreadedQueue<T>::isEmpty() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _critical.empty() && _high.empty() && _medium.empty() && _low.empty();
}

// Explicit instantiations
template class ThreadedQueue<MessageData>;
template class ThreadedQueue<DecodedMessage>;
