#ifndef ECS_SYSTEMS_SPRITE_HPP
#define ECS_SYSTEMS_SPRITE_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include "../registry.hpp"
#include "../components/sprite.hpp"
#include "../components/position.hpp"

class SpriteSystem {
    public:
        using TextureId = std::string;
        using DrawCallback = std::function<void(const TextureId&, int, int, int, int, int)>;

        SpriteSystem() = default;

        void render(Registry& reg, DrawCallback drawCallback);
};

#endif