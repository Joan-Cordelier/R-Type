#include "SettingMenu.hpp"
#include "../common/ecs/registry.hpp"

SettingMenu::SettingMenu(Registry& reg)
{
    DaltonianSlider = reg.createEntity();
    DaltonianMode = reg.createEntity();
    
    reg.addComponent<Slider>(DaltonianSlider, 0.0f, 0.0f, 100.0f, 150.0f, 20.0f, 0, 16, 24, false, false, false, std::string("DaltonianHandler"), Color(100, 100, 100), Color(50, 150, 250), Color(255, 255, 255));
    reg.addComponent<Position>(DaltonianSlider, 300.0f, 170.0f);

    reg.addComponent<Position>(DaltonianMode, 275.0f, 100.0f);
    reg.addComponent<SpriteSheets>(DaltonianMode, std::string(""), std::string("daltonian_btn"), 200, 50, 0, 3, 0, false, false);
    reg.addComponent<Sprite>(DaltonianMode, std::string("textures/colorblind_btn/colorblindbtn.png"), std::string("daltonian_btn"), 200, 50, 0, false);
    reg.addComponent<Button>(DaltonianMode, std::string("DaltonianModeButton"), 1, false);
}

void SettingMenu::setup(Registry& reg, SliderSystem& slidersys, ButtonSystem& buttonsys)
{
    (void) reg;
    (void) slidersys;
    buttonsys.registerHandler("DaltonianModeButton", [&](Registry& r, Entity e) {
        // Cycle through Daltonian modes
        switch (currentDaltonianMode) {
            case DaltonianType::None:
                currentDaltonianMode = DaltonianType::Protanopia;
                r.getComponent<SpriteSheets>(e).frameIndex = 1;
                std::cout << "Daltonian mode set to Protanopia" << std::endl;
                break;
            case DaltonianType::Protanopia:
                currentDaltonianMode = DaltonianType::Deuteranopia;
                r.getComponent<SpriteSheets>(e).frameIndex = 2;
                std::cout << "Daltonian mode set to Deuteranopia" << std::endl;
                break;
            case DaltonianType::Deuteranopia:
                currentDaltonianMode = DaltonianType::Tritanopia;
                r.getComponent<SpriteSheets>(e).frameIndex = 3;
                std::cout << "Daltonian mode set to Tritanopia" << std::endl;
                break;
            case DaltonianType::Tritanopia:
                currentDaltonianMode = DaltonianType::None;
                r.getComponent<SpriteSheets>(e).frameIndex = 0;
                std::cout << "Daltonian mode set to None" << std::endl;
                break;
        }
    });
}

void SettingMenu::toggle(Registry& reg)
{
    toggled = !toggled;
    updateVisibility(reg);
}

void SettingMenu::updateVisibility(Registry& reg)
{
    std::cout << "SettingMenu visibility set to " << (toggled ? "true" : "false") << std::endl;
    reg.getComponent<Slider>(DaltonianSlider).visible = toggled;
    reg.getComponent<Slider>(DaltonianSlider).enabled = toggled;
    reg.getComponent<Button>(DaltonianMode).visible = toggled;
    reg.getComponent<Button>(DaltonianMode).enabled = toggled;
    reg.getComponent<SpriteSheets>(DaltonianMode).visible = toggled;
}