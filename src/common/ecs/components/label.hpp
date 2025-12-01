#ifndef ECS_COMPONENTS_LABEL_HPP
#define ECS_COMPONENTS_LABEL_HPP

#include <string>
#include "../../graphic/Color.hpp"

struct Label {
    std::string text;
    std::string textName;
    std::string textIndex;
    Color color = Color(255, 255, 255);
    int z = 0;
    bool visible = true;
};

#endif