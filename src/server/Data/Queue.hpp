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
#include <mutex>
#include "packet.hpp"

class Queue {
    public:
        using PacketData = std::vector<uint8_t>;

        ~Queue();
        PacketData pop(Priority level);
        void push(Priority level, const PacketData& data);
    protected:
        std::mutex _mutex;
        std::queue<PacketData> _critical;
        std::queue<PacketData> _high;
        std::queue<PacketData> _medium;
        std::queue<PacketData> _low;
        
};

#endif /* !QUEUE_HPP_ */
