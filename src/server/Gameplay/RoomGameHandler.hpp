/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RoomGameHandler - GameHandler adapted for Room-based architecture
*/

#ifndef ROOMGAMEHANDLER_HPP_
#define ROOMGAMEHANDLER_HPP_

#include "../../common/Config/GameLoopConfig.hpp"
#include "../../common/Data/EntityType.hpp"
#include "../../common/ecs/systems/weapon_system.hpp"
#include "../common/Data/MessageFactory.hpp"
#include "../common/ecs/components/enemy.hpp"
#include "../common/ecs/components/label.hpp"
#include "../common/ecs/components/position.hpp"
#include "../common/ecs/components/projectile.hpp"
#include "../common/ecs/components/stats.hpp"
#include "../common/ecs/components/velocity.hpp"
#include "../common/ecs/components/weapon.hpp"
#include "ecs/registry.hpp"
#include "ecs/systems/enemy_system.hpp"
#include "ecs/systems/movement_system.hpp"
#include "ecs/systems/stat_system.hpp"

#include <atomic>
#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include <set>

struct RoomMoveData {
    uint32_t playerId;
    float vx;
    float vy;
};

struct RoomShootData {
    uint32_t playerId;
};

/**
 * @brief GameHandler adapted for Room-based architecture.
 *
 * Unlike the original GameHandler which uses SessionManager directly,
 * this version uses callbacks for sending messages, making it decoupled
 * from the network layer.
 */
class RoomGameHandler {
public:
    using SendCallback =
        std::function<void(uint32_t playerId, const PreparedMessage &msg)>;
    using BroadcastCallback = std::function<void(const PreparedMessage &msg)>;

    RoomGameHandler(std::atomic<bool> &running, SendCallback sendUdp,
                    SendCallback sendTcp, BroadcastCallback broadcastUdp,
                    BroadcastCallback broadcastTcp);
    ~RoomGameHandler() = default;

    // Main game loop (runs on dedicated thread)
    void run();

    // Player events (called from Room when routing messages)
    void onPlayerConnect(uint32_t playerId);
    void onPlayerDisconnect(uint32_t playerId);
    void onPlayerMove(const RoomMoveData &moveData);
    void onPlayerShoot(const RoomShootData &shootData);
    void onPlayerLinked(uint32_t playerId);

    // Check if game has minimum players to start
    bool hasMinimumPlayers() const;

    // Called by Room to update game state (single-threaded, no mutex needed)
    void updateGame(float deltaTime);
    void sendUpdatedPositionToAllPlayers();

private:
    std::atomic<bool> &_running;
    std::atomic<bool> _gameStarted{false};

    // Callbacks for sending
    SendCallback _sendUdp;
    SendCallback _sendTcp;
    BroadcastCallback _broadcastUdp;
    BroadcastCallback _broadcastTcp;

    // ECS
    Registry _reg;
    MovementSystem _movement;
    StatSystem _statSystem;

    EnemySystem _enemySystem;
    WeaponSystem _weaponSystem;

    // Player tracking
    std::map<uint32_t, Entity> _playerEntities;
    std::map<Entity, bool> _wasMoving;
    mutable std::mutex _playersMutex; // Protects _playerEntities, _wasMoving
    mutable std::mutex _regMutex; // Protects _reg for thread-safe ECS access

    Entity _scoreEntity;
    int _score = 0;

    // Game loop helpers
    void sendUpdatedPositionToPlayer(uint32_t playerId);
    void sendNewProjectilesToAllPlayers(Entity parentEntity,
                                        Entity projectileEntity);
    void sendDestroyedProjectileToAllPlayers(Entity projectile);
    void sendDestroyedEnemyToAllPlayers(Entity enemy);
    void sendDestroyedPlayerToAllPlayers(Entity player);
    void initNewEnemyEntities(const std::vector<Entity> &enemyEntities);
    void updateEnemyPosition(const std::vector<Entity> &enemyEntities);

    // Config
    GameLoopConfig _config;
    // Collision
    void checkPlayerCollisions();
};

#endif /* !ROOMGAMEHANDLER_HPP_ */
