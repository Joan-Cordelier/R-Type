/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** ThreadedQueue
*/

#ifndef THREADEDQUEUE_HPP_
#define THREADEDQUEUE_HPP_

#include <queue>
#include <vector>
#include <cstdint>
#include <mutex>
#include <optional>
#include "MessageFactory.hpp"

template<typename T>
class ThreadedQueue {
    public:
        ~ThreadedQueue();
        std::optional<T> pop(Priority level);
        void push(Priority level, const T& data);
        bool isEmpty(Priority level) const;
        bool isEmpty() const;
        
    protected:
        mutable std::mutex _mutex;
        std::queue<T> _critical;
        std::queue<T> _high;
        std::queue<T> _medium;
        std::queue<T> _low;
};

#endif /* !THREADEDQUEUE_HPP_ */
