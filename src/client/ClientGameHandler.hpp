#pragma once

#include "../common/Data/MessageFactory.hpp"
#include "Network/NetworkManager.hpp"
#include "graphic/Renderer.hpp"
#include "input_system.hpp"
#include "AudioManager.hpp"

#include "../common/Config/GameLoopConfig.hpp"
#include "../common/ecs/ecs_components.hpp"
#include "../common/ecs/ecs_system.hpp"
#include "../common/ecs/registry.hpp"
#include "../common/ecs/systems/slider_system.hpp"
#include "../common/ecs/systems/weapon_system.hpp"

#include "KeybindsManager.hpp"
#include "LoginMenu.hpp"
#include "LobbyMenu.hpp"
#include "CreateRoomMenu.hpp"
#include "SettingMenu.hpp"

enum class GameState {
    MAIN_MENU,
    LOGIN,
    LOBBY,
    CREATE_ROOM,
    IN_GAME
};
#include <SDL2/SDL.h>
#include <arpa/inet.h>
#include <functional>
#include <map>
#include <sys/socket.h>
#include <unordered_map>
#include <vector>

class ClientGameHandler {
private:
    Renderer _renderer;
    Registry _reg;
    MovementSystem _movement;
    SpriteSystem _spritesys;
    ButtonSystem _buttonsys;
    LabelSystem _labelsys;
    SpriteSheetSystem _spritesheetsys;
    StatSystem _statsys;
    WeaponSystem _weaponsys;
    InputSystem _input;
    SliderSystem _slidersys;

    SettingMenu _settingsMenu;
    KeybindsManager _keybindsManager;
    LoginMenu _loginMenu;
    LobbyMenu _lobbyMenu;
    CreateRoomMenu _createRoomMenu;

    NetworkManager _network;
    GameLoopConfig _config;

    GameState _gameState = GameState::MAIN_MENU;

    std::map<Entity, Entity> playerEntities;
    std::map<Entity, Entity> enemyEntities;
    std::map<Entity, Entity> projectileEntities;
    std::map<Entity, Entity> companionEntities;
    std::vector<DecodedMessage> pendingPlayerPackets;

    double animationClock = 0.0;
    bool running = true;
    bool settingsMenuOpen = false;

    std::string ip_adress = "127.0.0.1";
    bool _debugMode = false;
    bool _joinedRoom = false;
    std::vector<RoomInfo> _pendingRooms;

    uint32_t _userId = 0;
    std::string _username;
    bool _isGuest = false;
    bool _isAuthenticated = false;

    // menu Entities
    Entity start_button = _reg.createEntity();
    Entity label_input = _reg.createEntity();
    Entity background = _reg.createEntity();

    void toggleSettingsMenu();

    void handleMessages();
    void handlePlayerPacket(const DecodedMessage &msg);

    void cleanupServerEntity(Entity serverEntity);
    
    AudioManager _audioManager;

    void setupLobbyCallbacks();
    void setupLoginCallbacks();
    void requestRoomList();
    void showCreateRoomMenu();
    void createRoom(const RoomConfig& config);
    void joinRoom(uint32_t roomId);
    void registerJoinHandler(uint32_t roomId);

public:
    uint32_t myPlayerId = 0;
    Entity myEntity = 0;
    ClientGameHandler(bool debugMode = false);
    ~ClientGameHandler() = default;
    int run();

    // Upgrades
    bool upgradeMenuOpen = false;
    std::vector<Entity> upgradeMenuEntities;
    void handleUpgradeOptions(const DecodedMessage &msg);
    void handleUpdateWeapon(const DecodedMessage &msg);
    void showUpgradeMenu(const std::vector<std::string> &ids);
    void selectUpgrade(int index);
};