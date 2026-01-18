/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LobbyMenu - Client lobby UI for room selection and creation
*/

#ifndef LOBBYMENU_HPP_
#define LOBBYMENU_HPP_

#include "../common/ecs/registry.hpp"
#include "../common/ecs/ecs_components.hpp"
#include "../common/ecs/ecs_system.hpp"
#include <functional>
#include <string>
#include <vector>

struct RoomInfo {
    uint32_t id;
    uint8_t playerCount;
    uint8_t maxPlayers = 4;
};

class LobbyMenu {
public:
    using JoinRoomCallback = std::function<void(uint32_t roomId)>;
    using CreateRoomCallback = std::function<void()>;
    using RefreshCallback = std::function<void()>;

    LobbyMenu(Registry &reg);
    ~LobbyMenu() = default;

    void init();  // Call after textures are loaded
    void setup(ButtonSystem &buttonsys);
    void show();
    void hide();
    bool isVisible() const { return _visible; }

    void setUsername(const std::string &username, bool isGuest);
    void updateRoomList(const std::vector<RoomInfo> &rooms);
    void setJoinCallback(JoinRoomCallback callback) { _onJoinRoom = callback; }
    void setCreateCallback(CreateRoomCallback callback) { _onCreateRoom = callback; }
    void setRefreshCallback(RefreshCallback callback) { _onRefresh = callback; }

    void clearRoomEntities();

private:
    Registry &_reg;
    bool _visible = false;

    // UI Entities
    Entity _titleLabel;
    Entity _usernameLabel;
    Entity _createRoomButton;
    Entity _refreshButton;
    std::vector<Entity> _roomEntities;
    std::vector<Entity> _joinButtonEntities;

    // Room data
    std::vector<RoomInfo> _rooms;

    // Callbacks
    JoinRoomCallback _onJoinRoom;
    CreateRoomCallback _onCreateRoom;
    RefreshCallback _onRefresh;

    void createRoomEntry(size_t index, const RoomInfo &room, ButtonSystem &buttonsys);
};

#endif /* !LOBBYMENU_HPP_ */
