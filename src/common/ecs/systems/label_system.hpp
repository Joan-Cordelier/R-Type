/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** label_system
*/

#ifndef ECS_SYSTEMS_LABEL_SYSTEM_HPP
#define ECS_SYSTEMS_LABEL_SYSTEM_HPP

#include <string>
#include <unordered_map>
#include <functional>
#include "../registry.hpp"
#include "../components/label.hpp"
#include "../../../engine/graphic/Color.hpp"

class LabelSystem {
public:
    using TextId = std::string;
    using DrawCallback = std::function<void(const TextId&, std::string&, int, int, Color)>;

    LabelSystem();

    void render(Registry& reg, DrawCallback drawCallback);
};

#endif