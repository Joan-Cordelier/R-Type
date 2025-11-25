/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** main
*/

#include "Network/TCPServer.hpp"
#include "Network/UDPServer.hpp"
#include "Data/Queue.hpp"
#include <thread>
#include <iostream>
#include <csignal>
#include <atomic>

std::atomic<bool> running(true);

void signalHandler(int sig)
{
    (void)sig;
    running = false;
}

void processQueue(Queue& queue)
{
    while (running) {
        bool foundPacket = false;
        
        auto critical = queue.pop(Priority::CRITICAL);
        if (!(critical.size() == 1 && critical[0] == PARSING_ERROR)) {
            // Process critical priority packet
            foundPacket = true;
        }

        if (!foundPacket) {
            auto high = queue.pop(Priority::HIGH);
            if (!(high.size() == 1 && high[0] == PARSING_ERROR)) {
                // Process high priority packet
                foundPacket = true;
            }
        }

        if (!foundPacket) {
            auto medium = queue.pop(Priority::MEDIUM);
            if (!(medium.size() == 1 && medium[0] == PARSING_ERROR)) {
                // Process medium priority packet
                foundPacket = true;
            }
        }

        if (!foundPacket) {
            auto low = queue.pop(Priority::LOW);
            if (!(low.size() == 1 && low[0] == PARSING_ERROR)) {
                // Process low priority packet
                foundPacket = true;
            }
        }

        if (!foundPacket) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

int main()
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    Queue queue;
    TCPServer tcpServer(queue);
    UDPServer udpServer(queue);

    std::thread tcpThread(&TCPServer::run, &tcpServer);
    std::thread udpThread(&UDPServer::run, &udpServer);
    std::thread queueThread(processQueue, std::ref(queue));

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    tcpServer.stop();
    udpServer.stop();

    tcpThread.join();
    udpThread.join();
    queueThread.join();

    return 0;
}
