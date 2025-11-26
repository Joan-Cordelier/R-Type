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
    struct DrawCmd {
        std::string textureName;
        TextureId textureId;
        float x, y;
        int z;
    };

    SpriteSystem() = default;

    void registerTexture(const std::string& name, TextureId id);
    void unregisterTexture(const std::string& name);
    void render(Registry& reg, std::function<void(const TextureId&, float, float, int)> drawCallback);
    std::vector<DrawCmd> collectDrawCommands(Registry& reg) const;

private:
    std::unordered_map<std::string, TextureId> textures;
};

#endif