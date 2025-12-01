#include "entity_manager.hpp"

EntityManager::EntityManager() : nextId(1) {}

Entity EntityManager::create() {
    if (!freeIds.empty()) {
        Entity e = freeIds.back();
        freeIds.pop_back();
        return e;
    }
    return nextId++;
}

void EntityManager::destroy(Entity e) {
    if (e == INVALID_ENTITY) return;
    freeIds.push_back(e);
}