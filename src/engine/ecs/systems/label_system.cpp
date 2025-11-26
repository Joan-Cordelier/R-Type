/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** label_system
*/

#include "label_system.hpp"
#include "../components/position.hpp"
#include <iostream>

LabelSystem::LabelSystem() {}

void LabelSystem::render(Registry& reg, DrawCallback drawCallback) {
    if (!drawCallback) return;
    auto lblArr = reg.componentArray<Label>();
    if (!lblArr) return;
    auto posArr = reg.componentArray<Position>();

    for (auto e : lblArr->entities()) {
        if (!lblArr->has(e)) continue;
        auto &lbl = lblArr->get(e);
        if (!lbl.visible) continue;

        float x = 0.f, y = 0.f;
        if (posArr && posArr->has(e)) {
            auto &p = posArr->get(e);
            x = p.x; y = p.y;
        }

        drawCallback(lbl.textIndex, lbl.text, x, y, lbl.color);
    }
}
