#pragma once

#include "../common/Data/MessageFactory.hpp"
#include "AudioManager.hpp"
#include "Network/NetworkManager.hpp"
#include "graphic/Renderer.hpp"
#include "input_system.hpp"

#include "../common/Config/GameLoopConfig.hpp"
#include "../common/ecs/ecs_components.hpp"
#include "../common/ecs/ecs_system.hpp"
#include "../common/ecs/registry.hpp"
#include "../common/ecs/systems/slider_system.hpp"
#include "../common/ecs/systems/weapon_system.hpp"

#include "ChatPanel.hpp"
#include "CreateRoomMenu.hpp"
#include "EndGameScreen.hpp"
#include "KeybindsManager.hpp"
#include "LobbyMenu.hpp"
#include "LoginMenu.hpp"
#include "PauseMenu.hpp"
#include "ScoreboardMenu.hpp"
#include "SettingMenu.hpp"

enum class GameState { MAIN_MENU, LOGIN, LOBBY, CREATE_ROOM, SCOREBOARD, IN_GAME, DEAD };
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
    ChatPanel _chatPanel;
    ScoreboardMenu _scoreboardMenu;
    EndGameScreen _endGameScreen;
    PauseMenu _pauseMenu;

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
    bool pauseMenuOpen = false;
    bool _settingsOpenedFromPause = false;

    std::string ip_adress = "127.0.0.1";
    bool _debugMode = false;
    bool _joinedRoom = false;
    std::vector<RoomInfo> _pendingRooms;
    bool _chatVisibleBeforeMenu = false;

    uint32_t _userId = 0;
    std::string _username;
    bool _isGuest = false;
    bool _isAuthenticated = false;

    Entity _scoreLabel = 0;
    uint32_t _currentScore = 0;
    uint8_t _participantCount = 1; // For calculating final score (score / participants)

    // menu Entities
    Entity start_button = _reg.createEntity();
    Entity label_input = _reg.createEntity();
    Entity background = _reg.createEntity();

    void toggleSettingsMenu();
    void togglePauseMenu();
    void setupPauseMenuCallbacks();

    void handleMessages();
    void handlePlayerPacket(const DecodedMessage &msg);

    void cleanupServerEntity(Entity serverEntity);

    AudioManager _audioManager;

    void setupLobbyCallbacks();
    void setupLoginCallbacks();
    void setupChatCallbacks();
    void setupScoreboardCallbacks();
    void setupEndGameScreenCallbacks();
    void requestRoomList();
    void requestScoreboard();
    void requestLeaderboard(uint8_t difficulty);
    void showCreateRoomMenu();
    void createRoom(const RoomConfig &config);
    void joinRoom(uint32_t roomId);
    void registerJoinHandler(uint32_t roomId);
    void sendChatMessage(const std::string &message);
    void returnToLobby();
    void cleanupGameEntities();

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