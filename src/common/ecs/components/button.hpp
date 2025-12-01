#ifndef ECS_COMPONENTS_BUTTON_HPP
#define ECS_COMPONENTS_BUTTON_HPP

#include <string>

struct Button {
    std::string handler;
    int z = 0;
    bool enabled = true;
    bool visible = true;
};

#endif