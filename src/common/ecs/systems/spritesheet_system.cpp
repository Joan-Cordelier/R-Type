#include "spritesheet_system.hpp"
#include <iostream>

void SpriteSheetSystem::render(Registry& reg, std::function<void(const TextureId&, int, int, int, int, int, int)> drawCallback) {
    if (!drawCallback) return;
    auto spArr = reg.componentArray<SpriteSheets>();
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
        sp.frameIndex = (sp.frameIndex + 1) % sp.maxFrames;

        drawCallback(sp.textureIndex, sp.frameIndex, sp.width, sp.height, x, y, sp.z);
    }
}
