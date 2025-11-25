#include "sprite_system.hpp"

void SpriteSystem::registerTexture(const std::string& name, TextureId id) {
    textures[name] = id;
}

void SpriteSystem::unregisterTexture(const std::string& name) {
    textures.erase(name);
}

void SpriteSystem::render(Registry& reg, std::function<void(TextureId, const std::string&, float, float, int)> drawCallback) {
    if (!drawCallback) return;
    auto spArr = reg.componentArray<Sprite>();
    if (!spArr) return;
    auto posArr = reg.componentArray<Position>();

    for (auto e : spArr->entities()) {
        if (!spArr->has(e)) continue;
        auto &sp = spArr->get(e);
        if (!sp.visible) continue;

        float x = 0.f, y = 0.f;
        if (posArr && posArr->has(e)) {
            auto &p = posArr->get(e);
            x = p.x; y = p.y;
        }

        TextureId tid = "0";
        auto it = textures.find(sp.textureName);
        if (it != textures.end()) tid = it->second;

        drawCallback(tid, sp.textureName, x, y, sp.z);
    }
}

std::vector<SpriteSystem::DrawCmd> SpriteSystem::collectDrawCommands(Registry& reg) const {
    std::vector<DrawCmd> out;
    auto spArr = reg.componentArray<Sprite>();
    if (!spArr) return out;
    auto posArr = reg.componentArray<Position>();

    for (auto e : spArr->entities()) {
        if (!spArr->has(e)) continue;
        const auto &sp = spArr->get(e);
        if (!sp.visible) continue;
        float x = 0.f, y = 0.f;
        if (posArr && posArr->has(e)) {
            const auto &p = posArr->get(e);
            x = p.x; y = p.y;
        }
        std::string tid = "0";
        auto it = textures.find(sp.textureName);
        if (it != textures.end()) tid = it->second;
        out.push_back({sp.textureName, tid, x, y, sp.z});
    }
    return out;
}