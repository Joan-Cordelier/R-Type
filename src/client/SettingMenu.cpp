#include "SettingMenu.hpp"
#include "../common/ecs/registry.hpp"

SettingMenu::SettingMenu(Registry& reg, KeybindsManager& kbManager) : keybindsManager(kbManager)
{

    DaltonianSlider = reg.createEntity();
    DaltonianMode = reg.createEntity();
    MoveUpKeybindBtn = reg.createEntity();
    MoveDownKeybindBtn = reg.createEntity();
    MoveLeftKeybindBtn = reg.createEntity();
    MoveRightKeybindBtn = reg.createEntity();
    MoveUpLabel = reg.createEntity();
    MoveDownLabel = reg.createEntity();
    MoveLeftLabel = reg.createEntity();
    MoveRightLabel = reg.createEntity();
    
    reg.addComponent<Slider>(DaltonianSlider, 0.0f, 0.0f, 100.0f, 150.0f, 20.0f, 0, 16, 24, false, false, false, std::string("DaltonianHandler"), Color(100, 100, 100), Color(50, 150, 250), Color(255, 255, 255));
    reg.addComponent<Position>(DaltonianSlider, 300.0f, 170.0f);

    reg.addComponent<Position>(DaltonianMode, 275.0f, 100.0f);
    reg.addComponent<SpriteSheets>(DaltonianMode, std::string(""), std::string("daltonian_btn"), 200, 50, 0, 3, 0, false, false);
    reg.addComponent<Sprite>(DaltonianMode, std::string("textures/colorblind_btn/colorblindbtn.png"), std::string("daltonian_btn"), 200, 50, 0, false); //only for button system to work
    reg.addComponent<Button>(DaltonianMode, std::string("DaltonianModeButton"), 1, false);

    reg.addComponent<Position>(MoveUpKeybindBtn, 1000.0f, 170.0f);
    reg.addComponent<SpriteSheets>(MoveUpKeybindBtn, std::string(""), std::string("keybinds"), 50, 50, 38, 111, 0, false, false);
    reg.addComponent<Sprite>(MoveUpKeybindBtn, std::string("textures/keybinds_btn/colorblindbtn.png"), std::string("daltonian_btn"), 50, 50, 0, false); //only for button system to work
    reg.addComponent<Button>(MoveUpKeybindBtn, std::string("MoveUpKeybindButton"), 1, false);
    reg.addComponent<Position>(MoveUpLabel, 860.0f, 180.0f);
    reg.addComponent<Label>(MoveUpLabel, std::string("Move Up"), std::string(""), std::string("default_font_small"), Color(255, 255, 255), 1, false);

    reg.addComponent<Position>(MoveDownKeybindBtn, 1000.0f, 240.0f);
    reg.addComponent<SpriteSheets>(MoveDownKeybindBtn, std::string(""), std::string("keybinds"), 50, 50, 34, 111, 0, false, false);
    reg.addComponent<Sprite>(MoveDownKeybindBtn, std::string("textures/keybinds_btn/colorblindbtn.png"), std::string("daltonian_btn"), 50, 50, 0, false); //only for button system to work
    reg.addComponent<Button>(MoveDownKeybindBtn, std::string("MoveDownKeybindButton"), 1, false);
    reg.addComponent<Position>(MoveDownLabel, 860.0f, 250.0f);
    reg.addComponent<Label>(MoveDownLabel, std::string("Move Down"), std::string(""), std::string("default_font_small"), Color(255, 255, 255), 1, false);

    reg.addComponent<Position>(MoveLeftKeybindBtn, 1000.0f, 310.0f);
    reg.addComponent<SpriteSheets>(MoveLeftKeybindBtn, std::string(""), std::string("keybinds"), 50, 50, 16, 111, 0, false, false);
    reg.addComponent<Sprite>(MoveLeftKeybindBtn, std::string("textures/keybinds_btn/colorblindbtn.png"), std::string("daltonian_btn"), 50, 50, 0, false); //only for button system to work
    reg.addComponent<Button>(MoveLeftKeybindBtn, std::string("MoveLeftKeybindButton"), 1, false);
    reg.addComponent<Position>(MoveLeftLabel, 860.0f, 320.0f);
    reg.addComponent<Label>(MoveLeftLabel, std::string("Move Left"), std::string(""), std::string("default_font_small"), Color(255, 255, 255), 1, false);

    reg.addComponent<Position>(MoveRightKeybindBtn, 1000.0f, 380.0f);
    reg.addComponent<SpriteSheets>(MoveRightKeybindBtn, std::string(""), std::string("keybinds"), 50, 50, 19, 111, 0, false, false);
    reg.addComponent<Sprite>(MoveRightKeybindBtn, std::string("textures/keybinds_btn/colorblindbtn.png"), std::string("daltonian_btn"), 50, 50, 0, false); //only for button system to work
    reg.addComponent<Button>(MoveRightKeybindBtn, std::string("MoveRightKeybindButton"), 1, false);
    reg.addComponent<Position>(MoveRightLabel, 860.0f, 390.0f);
    reg.addComponent<Label>(MoveRightLabel, std::string("Move Right"), std::string(""), std::string("default_font_small"), Color(255, 255, 255), 1, false);
}

void SettingMenu::setup(Registry& reg, SliderSystem& slidersys, ButtonSystem& buttonsys)
{
    (void) reg;
    (void) slidersys;
    
    keybindsManager.setOnKeybindChangedCallback([&](KeybindAction action, SDL_Scancode scancode) {
        
    });
    
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

    reg.getComponent<Button>(MoveUpKeybindBtn).visible = toggled;
    reg.getComponent<Button>(MoveUpKeybindBtn).enabled = toggled;
    reg.getComponent<SpriteSheets>(MoveUpKeybindBtn).visible = toggled;
    reg.getComponent<Label>(MoveUpLabel).visible = toggled;

    reg.getComponent<Button>(MoveDownKeybindBtn).visible = toggled;
    reg.getComponent<Button>(MoveDownKeybindBtn).enabled = toggled;
    reg.getComponent<SpriteSheets>(MoveDownKeybindBtn).visible = toggled;
    reg.getComponent<Label>(MoveDownLabel).visible = toggled;

    reg.getComponent<Button>(MoveLeftKeybindBtn).visible = toggled;
    reg.getComponent<Button>(MoveLeftKeybindBtn).enabled = toggled;
    reg.getComponent<SpriteSheets>(MoveLeftKeybindBtn).visible = toggled;
    reg.getComponent<Label>(MoveLeftLabel).visible = toggled;

    reg.getComponent<Button>(MoveRightKeybindBtn).visible = toggled;
    reg.getComponent<Button>(MoveRightKeybindBtn).enabled = toggled;
    reg.getComponent<SpriteSheets>(MoveRightKeybindBtn).visible = toggled;
    reg.getComponent<Label>(MoveRightLabel).visible = toggled;
}