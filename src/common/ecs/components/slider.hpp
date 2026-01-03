/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** slider
*/

#ifndef ECS_COMPONENTS_SLIDER_HPP
#define ECS_COMPONENTS_SLIDER_HPP

#include <string>
#include <functional>
#include "../../../engine/graphic/Color.hpp"

struct Slider {
    float value = 0.0f; // Slider value (0.0 - 1.0)
    float minValue = 0.0f;
    float maxValue = 100.0f;
    float width = 150.0f;
    float height = 20.0f;
    int z;
    int handleWidth = 16;
    int handleHeight = 24;
    bool visible = true;
    bool enabled = true;
    bool isDragging = false;
    std::string handler;
    
    Color trackColor = Color(100, 100, 100);
    Color fillColor = Color(50, 150, 250);
    Color handleColor = Color(255, 255, 255);
    
    float getValue() const {
        return minValue + value * (maxValue - minValue);
    }
    
    void setValue(float val) {
        if (val < 0.0f) val = 0.0f;
        if (val > 1.0f) val = 1.0f;
        value = val;
    }
    
    void setScaledValue(float scaledVal) {
        if (maxValue == minValue) {
            value = 0.0f;
            return;
        }
        float normalized = (scaledVal - minValue) / (maxValue - minValue);
        setValue(normalized);
    }
};

#endif
