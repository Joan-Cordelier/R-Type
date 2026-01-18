/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LobbyMenu - Client lobby UI for room selection and creation
*/

#include "LobbyMenu.hpp"
#include <iostream>

LobbyMenu::LobbyMenu(Registry &reg) : _reg(reg) {
    // Entity creation moved to init() to ensure textures are loaded first
}

void LobbyMenu::init() {
    // Create title label
    _titleLabel = _reg.createEntity();
    _reg.addComponent<Position>(_titleLabel, 400.f, 50.f);
    _reg.addComponent<Label>(_titleLabel, std::string("Select a Room"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font"), Color(255, 255, 255), 100, false);

    // Create username label (top right corner)
    _usernameLabel = _reg.createEntity();
    _reg.addComponent<Position>(_usernameLabel, 750.f, 20.f);
    _reg.addComponent<Label>(_usernameLabel, std::string(""),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(200, 200, 200), 100, false);

    // Create "Create Room" button
    _createRoomButton = _reg.createEntity();
    _reg.addComponent<Position>(_createRoomButton, 125.f, 480.f);
    _reg.addComponent<Sprite>(_createRoomButton, std::string("textures/button/square_button.png"),
                              std::string("square_button"), 330, 160, 100, 0.f, 0.f, false);
    _reg.addComponent<Button>(_createRoomButton, std::string("lobby_create_room"), 100, false);

    // Create "Refresh" button
    _refreshButton = _reg.createEntity();
    _reg.addComponent<Position>(_refreshButton, 444.f, 480.f);
    _reg.addComponent<Sprite>(_refreshButton, std::string("textures/button/square_button.png"),
                              std::string("square_button"), 235, 160, 100, 0.f, 0.f, false);
    _reg.addComponent<Button>(_refreshButton, std::string("lobby_refresh"), 100, false);

    // Label for Create Room button
    Entity createLabel = _reg.createEntity();
    _reg.addComponent<Position>(createLabel, 210.f, 545.f);
    _reg.addComponent<Label>(createLabel, std::string("Create Room"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 255, 255), 101, false);
    _roomEntities.push_back(createLabel);

    // Label for Refresh button
    Entity refreshLabel = _reg.createEntity();
    _reg.addComponent<Position>(refreshLabel, 515.f, 545.f);
    _reg.addComponent<Label>(refreshLabel, std::string("Refresh"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 255, 255), 101, false);
    _roomEntities.push_back(refreshLabel);

    // Create "Scoreboard" button
    _scoreboardButton = _reg.createEntity();
    _reg.addComponent<Position>(_scoreboardButton, 668.f, 480.f);
    _reg.addComponent<Sprite>(_scoreboardButton, std::string("textures/button/square_button.png"),
                              std::string("square_button"), 280, 160, 100, 0.f, 0.f, false);
    _reg.addComponent<Button>(_scoreboardButton, std::string("lobby_scoreboard"), 100, false);

    // Label for Scoreboard button
    Entity scoreboardLabel = _reg.createEntity();
    _reg.addComponent<Position>(scoreboardLabel, 755.f, 545.f);
    _reg.addComponent<Label>(scoreboardLabel, std::string("Scoreboard"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 255, 255), 101, false);
    _roomEntities.push_back(scoreboardLabel);

    std::cout << "[LobbyMenu] Initialized" << std::endl;
}

void LobbyMenu::setup(ButtonSystem &buttonsys) {
    buttonsys.registerHandler("lobby_create_room", [this]([[maybe_unused]] Registry &r, [[maybe_unused]] Entity e) {
        if (_onCreateRoom && _visible) {
            std::cout << "[LobbyMenu] Create Room clicked" << std::endl;
            _onCreateRoom();
        }
    });

    buttonsys.registerHandler("lobby_refresh", [this]([[maybe_unused]] Registry &r, [[maybe_unused]] Entity e) {
        if (_onRefresh && _visible) {
            std::cout << "[LobbyMenu] Refresh clicked" << std::endl;
            _onRefresh();
        }
    });

    buttonsys.registerHandler("lobby_scoreboard", [this]([[maybe_unused]] Registry &r, [[maybe_unused]] Entity e) {
        if (_onScoreboard && _visible) {
            std::cout << "[LobbyMenu] Scoreboard clicked" << std::endl;
            _onScoreboard();
        }
    });
}

void LobbyMenu::show() {
    _visible = true;
    std::cout << "[LobbyMenu] Showing lobby menu" << std::endl;

    // Show title
    _reg.getComponent<Label>(_titleLabel).visible = true;

    // Show username label
    _reg.getComponent<Label>(_usernameLabel).visible = true;

    // Show buttons
    _reg.getComponent<Sprite>(_createRoomButton).visible = true;
    _reg.getComponent<Button>(_createRoomButton).enabled = true;
    _reg.getComponent<Sprite>(_refreshButton).visible = true;
    _reg.getComponent<Button>(_refreshButton).enabled = true;
    _reg.getComponent<Sprite>(_scoreboardButton).visible = true;
    _reg.getComponent<Button>(_scoreboardButton).enabled = true;

    // Show static room entities (button labels)
    std::cout << "[LobbyMenu] Showing " << _roomEntities.size() << " room entities" << std::endl;
    for (auto &entity : _roomEntities) {
        if (_reg.hasComponent<Label>(entity)) {
            _reg.getComponent<Label>(entity).visible = true;
        }
        if (_reg.hasComponent<Sprite>(entity)) {
            _reg.getComponent<Sprite>(entity).visible = true;
        }
        if (_reg.hasComponent<Button>(entity)) {
            _reg.getComponent<Button>(entity).enabled = true;
        }
    }

    // Show join buttons
    std::cout << "[LobbyMenu] Showing " << _joinButtonEntities.size() << " join button entities"
              << std::endl;
    for (auto &entity : _joinButtonEntities) {
        if (_reg.hasComponent<Label>(entity)) {
            _reg.getComponent<Label>(entity).visible = true;
        }
        if (_reg.hasComponent<Sprite>(entity)) {
            _reg.getComponent<Sprite>(entity).visible = true;
        }
        if (_reg.hasComponent<Button>(entity)) {
            _reg.getComponent<Button>(entity).enabled = true;
        }
    }
}

void LobbyMenu::hide() {
    _visible = false;

    // Hide title
    _reg.getComponent<Label>(_titleLabel).visible = false;

    // Hide username label
    _reg.getComponent<Label>(_usernameLabel).visible = false;

    // Hide buttons
    _reg.getComponent<Sprite>(_createRoomButton).visible = false;
    _reg.getComponent<Button>(_createRoomButton).enabled = false;
    _reg.getComponent<Sprite>(_refreshButton).visible = false;
    _reg.getComponent<Button>(_refreshButton).enabled = false;
    _reg.getComponent<Sprite>(_scoreboardButton).visible = false;
    _reg.getComponent<Button>(_scoreboardButton).enabled = false;

    // Hide all room entities
    for (auto &entity : _roomEntities) {
        if (_reg.hasComponent<Label>(entity)) {
            _reg.getComponent<Label>(entity).visible = false;
        }
        if (_reg.hasComponent<Sprite>(entity)) {
            _reg.getComponent<Sprite>(entity).visible = false;
        }
        if (_reg.hasComponent<Button>(entity)) {
            _reg.getComponent<Button>(entity).enabled = false;
        }
    }

    for (auto &entity : _joinButtonEntities) {
        if (_reg.hasComponent<Label>(entity)) {
            _reg.getComponent<Label>(entity).visible = false;
        }
        if (_reg.hasComponent<Sprite>(entity)) {
            _reg.getComponent<Sprite>(entity).visible = false;
        }
        if (_reg.hasComponent<Button>(entity)) {
            _reg.getComponent<Button>(entity).enabled = false;
        }
    }
}

void LobbyMenu::clearRoomEntities() {
    // Keep the first three entities (button labels: Create Room, Refresh, Scoreboard), destroy the rest
    while (_roomEntities.size() > 3) {
        _reg.destroyEntity(_roomEntities.back());
        _roomEntities.pop_back();
    }

    // Clear join button entities
    for (auto &entity : _joinButtonEntities) {
        _reg.destroyEntity(entity);
    }
    _joinButtonEntities.clear();
}

void LobbyMenu::updateRoomList(const std::vector<RoomInfo> &rooms) {
    _rooms = rooms;

    std::cout << "[LobbyMenu] Updating room list with " << rooms.size() << " rooms, visible="
              << _visible << std::endl;

    // Clear old room entities (keep button labels)
    clearRoomEntities();

    if (rooms.empty()) {
        // Show "No rooms available" message
        std::cout << "[LobbyMenu] No rooms - showing 'Create one' message" << std::endl;
        Entity noRoomsLabel = _reg.createEntity();
        _reg.addComponent<Position>(noRoomsLabel, 350.f, 150.f);
        _reg.addComponent<Label>(noRoomsLabel, std::string("No rooms available. Create one!"),
                                 std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                                 std::string("default_font_small"), Color(200, 200, 200), 100,
                                 _visible);
        _roomEntities.push_back(noRoomsLabel);
        return;
    }

    // Create entries for each room
    for (size_t i = 0; i < rooms.size() && i < 6; ++i) {
        float yPos = 120.f + i * 60.f;

        std::cout << "[LobbyMenu] Creating entry for room " << rooms[i].id << " at y=" << yPos
                  << std::endl;

        // Room info label
        Entity roomLabel = _reg.createEntity();
        _reg.addComponent<Position>(roomLabel, 150.f, yPos + 10.f);

        std::string roomText = "Room " + std::to_string(rooms[i].id) + " - " +
                               std::to_string(rooms[i].playerCount) + "/" +
                               std::to_string(rooms[i].maxPlayers) + " players";

        _reg.addComponent<Label>(roomLabel, roomText,
                                 std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                                 std::string("default_font_small"), Color(255, 255, 255), 100,
                                 _visible);
        _roomEntities.push_back(roomLabel);

        // Join button for this room
        if (rooms[i].playerCount < rooms[i].maxPlayers) {
            Entity joinButton = _reg.createEntity();
            _reg.addComponent<Position>(joinButton, 475.f, yPos - 34.f);
            _reg.addComponent<Sprite>(joinButton, std::string("textures/button/square_button.png"),
                                      std::string("square_button"), 160, 120, 100, 0.f, 0.f, _visible);

            std::string handlerName = "lobby_join_" + std::to_string(rooms[i].id);
            _reg.addComponent<Button>(joinButton, handlerName, 100, _visible);
            _joinButtonEntities.push_back(joinButton);

            // Join button label
            Entity joinLabel = _reg.createEntity();
            _reg.addComponent<Position>(joinLabel, 530.f, yPos + 12.f);
            _reg.addComponent<Label>(joinLabel, std::string("Join"),
                                     std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                                     std::string("default_font_small"), Color(255, 255, 255), 101,
                                     _visible);
            _joinButtonEntities.push_back(joinLabel);
        } else {
            // Room is full - show "Full" label
            Entity fullLabel = _reg.createEntity();
            _reg.addComponent<Position>(fullLabel, 530.f, yPos + 12.f);
            _reg.addComponent<Label>(fullLabel, std::string("Full"),
                                     std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                                     std::string("default_font_small"), Color(150, 150, 150), 100,
                                     _visible);
            _roomEntities.push_back(fullLabel);
        }
    }
    std::cout << "[LobbyMenu] Room list update complete" << std::endl;
}

void LobbyMenu::createRoomEntry([[maybe_unused]] size_t index, const RoomInfo &room, ButtonSystem &buttonsys) {
    uint32_t roomId = room.id;
    std::string handlerName = "lobby_join_" + std::to_string(roomId);

    buttonsys.registerHandler(handlerName, [this, roomId]([[maybe_unused]] Registry &r, [[maybe_unused]] Entity e) {
        if (_onJoinRoom && _visible) {
            std::cout << "[LobbyMenu] Joining room " << roomId << std::endl;
            _onJoinRoom(roomId);
        }
    });
}

void LobbyMenu::setUsername(const std::string &username, bool isGuest) {
    if (_reg.hasComponent<Label>(_usernameLabel)) {
        Label &label = _reg.getComponent<Label>(_usernameLabel);
        if (isGuest) {
            label.text = username + " (Guest)";
            label.color = Color(150, 150, 150);  // Gray for guests
        } else {
            label.text = username;
            label.color = Color(100, 255, 100);  // Green for registered users
        }
        std::cout << "[LobbyMenu] Username set to: " << label.text << std::endl;
    }
}
