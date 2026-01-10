#ifndef GAME_LOOP_CONFIG_HPP
#define GAME_LOOP_CONFIG_HPP

#include <string>
#include <yaml-cpp/yaml.h>
#include "../../common/Data/LevelData.hpp"
#include <vector>

struct UpgradeEffect {
    std::string target; // "max_health", "movement_speed", etc.
    float value;
};

struct UpgradeData {
    std::string id;
    std::string name;
    std::string description;
    std::string type; // "stat_boost", "hybrid"
    std::string rarity;
    std::vector<UpgradeEffect> effects;
    std::string icon_path;
    std::string card_color;
};

struct PlayerStatsConfig {
    int health = 100;
    int max_health = 100;
    int attack_damage = 10;
    int attack_speed = 5;
    int defense = 1;
    int speed = 200;
};

struct PlayerHitboxConfig {
    float width = 60.0f;
    float height = 30.0f;
    float offset_x = 30.0f;
    float offset_y = 50.0f;
    float sprite_width = 120.0f;
    float sprite_height = 130.0f;
};

struct PlayerConfig {
    float initial_x = 100.0f;
    float initial_y = 300.0f;
    PlayerHitboxConfig hitbox;
    PlayerStatsConfig stats;
};

struct EnemySpawningConfig {
    bool enabled = true;
    float initial_delay = 2.0f;
    float spawn_interval = 3.0f;
    int max_enemies = 10;
};

using EnemyTypeConfig = EnemyTypeData;
using EnemyWaveGroup = EnemyWaveGroupData;
using WaveConfig = WaveData;
using LevelConfig = LevelData;

struct ProjectileConfig {
    float speed = 400.0f;
    float offset_x = 52.0f;
    float offset_y = 30.0f;
};

struct ProjectilesConfig {
    ProjectileConfig player;
    ProjectileConfig enemy;
};

struct SystemsConfig {
    bool movement = true;
    bool stats = true;
    bool enemy = true;
    bool collision = true;
};

struct GameLoopSettings {
    int tick_rate = 30;
    int max_messages_per_frame = 100;
    int min_players_to_start = 1;
};

struct BossPhase {
    int id;
    std::string name;
    int trigger_health_percentage;
    std::vector<std::string> patterns;
    bool loop_patterns = false;
    std::string visual_effect;
};

struct BossAttackPattern {
    std::string id;
    std::string type;
    std::string projectile_id;
    int count;
    float spread_angle;
    float duration;
    float damage;
    float speed;
    float warning_time;
    std::string animation_trigger;
    std::string texture_override;
};

struct BossAnimation {
    int start_x;
    int start_y;
    int width;
    int height;
    int frame_count;
    float frame_duration;
    bool loop;
};

struct BossVisuals {
    std::string texture_path;
    float scale = 1.0f;
    float width = 100.0f;
    float height = 100.0f;
    float collider_width = 100.0f;
    float collider_height = 100.0f;
    float offset_x = 0.0f;
    float offset_y = 0.0f;
    std::map<std::string, BossAnimation> animations;
};

struct BossConfig {
    std::string id;
    std::string name;
    int health;
    int max_health;
    float speed;
    BossVisuals visuals;
    std::vector<BossPhase> phases;
    std::vector<BossAttackPattern> patterns;
};

class GameLoopConfig {
public:
    GameLoopConfig();
    ~GameLoopConfig() = default;
    
    bool loadFromFile(const std::string& filepath);
    bool loadUpgradesFromFile(const std::string& filepath);
    bool loadBossesFromDirectory(const std::string& dirPath);
    
    // Getters
    const GameLoopSettings& getGameLoopSettings() const { return _gameLoop; }
    const EnemySpawningConfig& getEnemySpawning() const { return _enemySpawning; }
    const PlayerConfig& getPlayerConfig() const { return _player; }
    const ProjectilesConfig& getProjectilesConfig() const { return _projectiles; }
    const SystemsConfig& getSystemsConfig() const { return _systems; }
    const std::map<std::string, EnemyTypeConfig>& getEnemyTypes() const { return _enemyTypes; }
    const std::map<std::string, BossConfig>& getBosses() const { return _bosses; }
    const std::vector<LevelConfig>& getLevels() const { return _levels; }
    const std::vector<UpgradeData>& getUpgrades() const { return _upgrades; }
    
    // Convenience methods
    float getTargetFrameTime() const { return 1.0f / static_cast<float>(_gameLoop.tick_rate); }
    int getMaxMessagesPerFrame() const { return _gameLoop.max_messages_per_frame; }
    int getMinPlayersToStart() const { return _gameLoop.min_players_to_start; }

private:
    void loadDefaults();
    
    GameLoopSettings _gameLoop;
    EnemySpawningConfig _enemySpawning;
    PlayerConfig _player;
    ProjectilesConfig _projectiles;
    SystemsConfig _systems;
    std::map<std::string, EnemyTypeConfig> _enemyTypes;
    std::map<std::string, BossConfig> _bosses;
    std::vector<LevelConfig> _levels;
    std::vector<UpgradeData> _upgrades;
};

#endif // GAME_LOOP_CONFIG_HPP