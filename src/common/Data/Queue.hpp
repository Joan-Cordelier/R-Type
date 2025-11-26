/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** Queue
*/

#ifndef QUEUE_HPP_
#define QUEUE_HPP_

#include <queue>
#include <vector>
#include <cstdint>
#include "MessageFactory.hpp"

class Queue {
    public:
        ~Queue();
        MessageData pop(Priority level);
        void push(Priority level, const MessageData& data);
        bool isEmpty(Priority level) const;
        bool isEmpty() const;
    private:
        std::queue<MessageData> _critical;
        std::queue<MessageData> _high;
        std::queue<MessageData> _medium;
        std::queue<MessageData> _low;
};

#endif /* !QUEUE_HPP_ */
