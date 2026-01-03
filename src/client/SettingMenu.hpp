#pragma once

#include "graphic/Renderer.hpp"
#include "input_system.hpp"

#include "../common/ecs/registry.hpp"
#include "../common/ecs/ecs_system.hpp"
#include "../common/ecs/ecs_components.hpp"
#include "../common/ecs/systems/slider_system.hpp"

class SettingMenu {
private:
    Entity DaltonianSlider;
    Entity DaltonianMode;
    DaltonianType currentDaltonianMode = DaltonianType::None;
public:
    bool toggled = false;
    SettingMenu(Registry& reg);
    void setup(Registry& reg, SliderSystem& slidersys, ButtonSystem& buttonsys);
    void toggle(Registry& reg);
    void updateVisibility(Registry& reg);
    DaltonianType getCurrentDaltonianMode() const { return currentDaltonianMode; }
    float getDaltonianSliderValue(Registry& reg) const {
        if (reg.hasComponent<Slider>(DaltonianSlider)) {
            return reg.getComponent<Slider>(DaltonianSlider).getValue();
        }
        return 0.0f;
    }
};