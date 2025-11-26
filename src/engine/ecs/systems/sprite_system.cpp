#include "sprite_system.hpp"

void SpriteSystem::render(Registry& reg, std::function<void(const TextureId&, int, int, int, int, int)> drawCallback) {
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

        drawCallback(sp.textureIndex, sp.width, sp.height, x, y, sp.z);
    }
}
