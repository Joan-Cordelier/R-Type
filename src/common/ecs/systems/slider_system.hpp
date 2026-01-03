/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** slider_system
*/

#ifndef ECS_SYSTEMS_SLIDER_SYSTEM_HPP
#define ECS_SYSTEMS_SLIDER_SYSTEM_HPP

#include <string>
#include <functional>
#include <unordered_map>
#include "../registry.hpp"

class SliderSystem {
    public:
        using Handler = std::function<void(Registry&, Entity, float)>;

        SliderSystem();

        void registerHandler(const std::string& name, Handler h);
        void unregisterHandler(const std::string& name);

        void update(Registry& reg);
        void render(Registry& reg, class Renderer& renderer);

    private:
        std::unordered_map<std::string, Handler> handlers;
        bool prevMouseDown;
        Entity draggedSlider;
};

#endif
