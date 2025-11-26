/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** button_system
*/

#ifndef ECS_SYSTEMS_BUTTON_SYSTEM_HPP
#define ECS_SYSTEMS_BUTTON_SYSTEM_HPP

#include <string>
#include <functional>
#include <unordered_map>
#include "../registry.hpp"

class ButtonSystem {
public:
    using Handler = std::function<void(Registry&, Entity)>;

    ButtonSystem();

    void registerHandler(const std::string& name, Handler h);
    void unregisterHandler(const std::string& name);

    void update(Registry& reg);

private:
    std::unordered_map<std::string, Handler> handlers;
    bool prevMouseDown;
};

#endif