/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Client main entry point
*/

#include "ClientGameHandler.hpp"
#include <string>
#include <vector>

int main(int argc, char* argv[])
{
    bool debugMode = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--debug") {
            debugMode = true;
        }
    }

    ClientGameHandler gameHandler(debugMode);
    return gameHandler.run();
}
