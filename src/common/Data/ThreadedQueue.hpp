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
#include "MessageFactory.hpp"

class ThreadedQueue {
    public:
        ~ThreadedQueue();
        MessageData pop(Priority level);
        void push(Priority level, const MessageData& data);
        bool isEmpty(Priority level) const;
        bool isEmpty() const;
    protected:
        mutable std::mutex _mutex;
        std::queue<MessageData> _critical;
        std::queue<MessageData> _high;
        std::queue<MessageData> _medium;
        std::queue<MessageData> _low;
        
};

#endif /* !THREADEDQUEUE_HPP_ */
