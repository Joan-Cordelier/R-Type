/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** slider_system
*/

#include "slider_system.hpp"
#include "../components/slider.hpp"
#include "../components/position.hpp"
#include "graphic/Renderer.hpp"
#include <SDL2/SDL.h>
#include <algorithm>
#include <iostream>

SliderSystem::SliderSystem() : prevMouseDown(false), draggedSlider(0) {}

void SliderSystem::registerHandler(const std::string& name, Handler h) {
    handlers[name] = std::move(h);
}

void SliderSystem::unregisterHandler(const std::string& name) {
    handlers.erase(name);
}

void SliderSystem::update(Registry& reg) {
    int mx, my;
    Uint32 buttons = SDL_GetMouseState(&mx, &my);
    bool mouseDown = (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;

    auto candidates = reg.viewEntitiesWith<Slider, Position>();

    // Handle dragging
    if (draggedSlider != 0 && mouseDown) {
        // Continue dragging the current slider
        if (reg.hasComponent<Slider>(draggedSlider) && reg.hasComponent<Position>(draggedSlider)) {
            auto& slider = reg.getComponent<Slider>(draggedSlider);
            const auto& pos = reg.getComponent<Position>(draggedSlider);

            if (slider.enabled && slider.visible) {
                float relativeX = mx - pos.x;
                float newValue = relativeX / slider.width;
                
                // Clamp the value
                if (newValue < 0.0f) newValue = 0.0f;
                if (newValue > 1.0f) newValue = 1.0f;

                float oldValue = slider.value;
                slider.value = newValue;

                // Call handler if value changed
                if (oldValue != newValue && !slider.handler.empty()) {
                    auto it = handlers.find(slider.handler);
                    if (it != handlers.end()) {
                        it->second(reg, draggedSlider, slider.getValue());
                    }
                }
            }
        }
    } else if (!mouseDown) {
        // Released mouse, stop dragging
        if (draggedSlider != 0) {
            if (reg.hasComponent<Slider>(draggedSlider)) {
                auto& slider = reg.getComponent<Slider>(draggedSlider);
                slider.isDragging = false;
            }
            draggedSlider = 0;
        }
    }

    // Check for new click on sliders
    if (!prevMouseDown && mouseDown && draggedSlider == 0) {
        // Sort by z-order
        std::sort(candidates.begin(), candidates.end(), [&](Entity a, Entity b) {
            const auto& sa = reg.getComponent<Slider>(a);
            const auto& sb = reg.getComponent<Slider>(b);
            return sa.z > sb.z;
        });

        for (auto e : candidates) {
            auto& slider = reg.getComponent<Slider>(e);
            if (!slider.enabled || !slider.visible) continue;

            if (!reg.hasComponent<Position>(e)) continue;
            const auto& pos = reg.getComponent<Position>(e);

            // Check if click is on the slider track or handle
            float left = pos.x;
            float top = pos.y;
            float right = left + slider.width;
            float bottom = top + slider.height;

            if (mx >= left && mx < right && my >= top && my < bottom) {
                // Start dragging this slider
                draggedSlider = e;
                slider.isDragging = true;

                // Set the value immediately to where clicked
                float relativeX = mx - pos.x;
                float newValue = relativeX / slider.width;
                
                // Clamp the value
                if (newValue < 0.0f) newValue = 0.0f;
                if (newValue > 1.0f) newValue = 1.0f;

                slider.value = newValue;

                // Call handler
                if (!slider.handler.empty()) {
                    auto it = handlers.find(slider.handler);
                    if (it != handlers.end()) {
                        it->second(reg, e, slider.getValue());
                    }
                }
                break; // Only interact with the topmost slider
            }
        }
    }

    prevMouseDown = mouseDown;
}

void SliderSystem::render(Registry& reg, Renderer& renderer) {
    auto sliders = reg.viewEntitiesWith<Slider, Position>();

    // Sort by z-order
    std::sort(sliders.begin(), sliders.end(), [&](Entity a, Entity b) {
        const auto& sa = reg.getComponent<Slider>(a);
        const auto& sb = reg.getComponent<Slider>(b);
        return sa.z < sb.z;
    });

    for (auto e : sliders) {
        const auto& slider = reg.getComponent<Slider>(e);
        if (!slider.visible) continue;

        const auto& pos = reg.getComponent<Position>(e);

        // Draw track background
        Rect trackRect = {
            static_cast<int>(pos.x),
            static_cast<int>(pos.y),
            slider.width,
            slider.height
        };
        renderer.drawRect(trackRect, slider.trackColor, RenderLayer::OVERLAY, slider.z, true);

        // Draw filled portion of track
        int fillWidth = static_cast<int>(slider.width * slider.value);
        if (fillWidth > 0) {
            Rect fillRect = {
                static_cast<int>(pos.x),
                static_cast<int>(pos.y),
                fillWidth,
                slider.height
            };
            renderer.drawRect(fillRect, slider.fillColor, RenderLayer::OVERLAY, slider.z + 1, true);
        }

        // Draw handle
        int handleX = static_cast<int>(pos.x + slider.width * slider.value - slider.handleWidth / 2);
        int handleY = static_cast<int>(pos.y + slider.height / 2 - slider.handleHeight / 2);
        
        Rect handleRect = {
            handleX,
            handleY,
            slider.handleWidth,
            slider.handleHeight
        };
        renderer.drawRect(handleRect, slider.handleColor, RenderLayer::OVERLAY, slider.z + 2, true);

        // Draw handle border
        Color borderColor(
            static_cast<uint8_t>(slider.handleColor.r * 0.7f),
            static_cast<uint8_t>(slider.handleColor.g * 0.7f),
            static_cast<uint8_t>(slider.handleColor.b * 0.7f)
        );
        renderer.drawRect(handleRect, borderColor, RenderLayer::OVERLAY, slider.z + 3, false);
    }
}
