#include "GameLoopConfig.hpp"
#include <fstream>
#include <iostream>
#include <dirent.h>
#include <sys/types.h>

GameLoopConfig::GameLoopConfig()
{
    loadDefaults();
}

void GameLoopConfig::loadDefaults()
{
    _gameLoop = GameLoopSettings{};
    _enemySpawning = EnemySpawningConfig{};
    _player = PlayerConfig{};
    _projectiles = ProjectilesConfig{};
    _systems = SystemsConfig{};
    
    // Default enemy type
    EnemyTypeConfig basicEnemy;
    basicEnemy.name = "basic";
    basicEnemy.texture_id = "enemy_ship";
    basicEnemy.sprite_path = "textures/ships/enemy_ship.png";
    _enemyTypes["basic"] = basicEnemy;
}

bool GameLoopConfig::loadFromFile(const std::string& filepath)
{
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Could not open config file: " << filepath << ", using defaults" << std::endl;
            return false;
        }
        
        YAML::Node config = YAML::LoadFile(filepath);
        
        // Parse game_loop section
        if (config["game_loop"]) {
            auto gl = config["game_loop"];
            if (gl["tick_rate"]) _gameLoop.tick_rate = gl["tick_rate"].as<int>();
            if (gl["max_messages_per_frame"]) _gameLoop.max_messages_per_frame = gl["max_messages_per_frame"].as<int>();
            if (gl["min_players_to_start"]) _gameLoop.min_players_to_start = gl["min_players_to_start"].as<int>();
        }
        
        // Parse enemy_spawning section
        if (config["enemy_spawning"]) {
            auto es = config["enemy_spawning"];
            if (es["enabled"]) _enemySpawning.enabled = es["enabled"].as<bool>();
            if (es["initial_delay"]) _enemySpawning.initial_delay = es["initial_delay"].as<float>();
            if (es["spawn_interval"]) _enemySpawning.spawn_interval = es["spawn_interval"].as<float>();
            if (es["max_enemies"]) _enemySpawning.max_enemies = es["max_enemies"].as<int>();
        }

        // Parse enemy_types section
        if (config["enemy_types"]) {
            auto ets = config["enemy_types"];
            for (auto it = ets.begin(); it != ets.end(); ++it) {
                std::string key = it->first.as<std::string>();
                auto node = it->second;
                EnemyTypeConfig typeConfig;
                typeConfig.name = key;
                if (node["sprite"]) typeConfig.sprite_path = node["sprite"].as<std::string>();
                if (node["texture_id"]) typeConfig.texture_id = node["texture_id"].as<std::string>();
                if (node["health"]) typeConfig.health = node["health"].as<int>();
                if (node["damage"]) typeConfig.damage = node["damage"].as<int>();
                if (node["speed"]) typeConfig.speed = node["speed"].as<int>();
                if (node["score"]) typeConfig.score = node["score"].as<int>();
                if (node["width"]) typeConfig.width = node["width"].as<float>();
                if (node["height"]) typeConfig.height = node["height"].as<float>();
                if (node["velocity_x"]) typeConfig.velocity_x = node["velocity_x"].as<float>();
                if (node["velocity_y"]) typeConfig.velocity_y = node["velocity_y"].as<float>();
                if (node["move_amplitude"]) typeConfig.move_amplitude = node["move_amplitude"].as<float>();
                if (node["move_frequency"]) typeConfig.move_frequency = node["move_frequency"].as<float>();
                _enemyTypes[key] = typeConfig;
            }
        }
        
        // Parse levels section
        if (config["levels"]) {
            auto levelsNode = config["levels"];
            for (std::size_t i = 0; i < levelsNode.size(); ++i) {
                auto lvlNode = levelsNode[i];
                LevelConfig level;
                if (lvlNode["id"]) level.id = lvlNode["id"].as<int>();
                if (lvlNode["name"]) level.name = lvlNode["name"].as<std::string>();
                if (lvlNode["background"]) level.background_path = lvlNode["background"].as<std::string>();
                if (lvlNode["music"]) level.music_path = lvlNode["music"].as<std::string>();
                
                if (lvlNode["waves"]) {
                    auto wavesNode = lvlNode["waves"];
                    for (std::size_t j = 0; j < wavesNode.size(); ++j) {
                        auto waveNode = wavesNode[j];
                        WaveConfig wave;
                        if (waveNode["id"]) wave.id = waveNode["id"].as<int>();
                        if (waveNode["start_delay"]) wave.start_delay = waveNode["start_delay"].as<float>();
                        if (waveNode["boss"]) wave.boss_id = waveNode["boss"].as<std::string>();
                        
                        if (waveNode["groups"]) {
                            auto groupsNode = waveNode["groups"];
                            for (std::size_t k = 0; k < groupsNode.size(); ++k) {
                                auto groupNode = groupsNode[k];
                                EnemyWaveGroup group;
                                if (groupNode["type"]) group.type = groupNode["type"].as<std::string>();
                                if (groupNode["count"]) group.count = groupNode["count"].as<int>();
                                if (groupNode["interval"]) group.interval = groupNode["interval"].as<float>();
                                wave.groups.push_back(group);
                            }
                        }
                        level.waves.push_back(wave);
                    }
                }
                _levels.push_back(level);
            }
        }
        
        // Parse player section
        if (config["player"]) {
            auto p = config["player"];
            if (p["initial_position"]) {
                if (p["initial_position"]["x"]) _player.initial_x = p["initial_position"]["x"].as<float>();
                if (p["initial_position"]["y"]) _player.initial_y = p["initial_position"]["y"].as<float>();
            }
            if (p["hitbox"]) {
                if (p["hitbox"]["width"]) _player.hitbox.width = p["hitbox"]["width"].as<float>();
                if (p["hitbox"]["height"]) _player.hitbox.height = p["hitbox"]["height"].as<float>();
                if (p["hitbox"]["offset_x"]) _player.hitbox.offset_x = p["hitbox"]["offset_x"].as<float>();
                if (p["hitbox"]["offset_y"]) _player.hitbox.offset_y = p["hitbox"]["offset_y"].as<float>();
                if (p["hitbox"]["sprite_width"]) _player.hitbox.sprite_width = p["hitbox"]["sprite_width"].as<float>();
                if (p["hitbox"]["sprite_height"]) _player.hitbox.sprite_height = p["hitbox"]["sprite_height"].as<float>();
            }
            if (p["stats"]) {
                auto s = p["stats"];
                if (s["health"]) _player.stats.health = s["health"].as<int>();
                if (s["max_health"]) _player.stats.max_health = s["max_health"].as<int>();
                if (s["attack_damage"]) _player.stats.attack_damage = s["attack_damage"].as<int>();
                if (s["attack_speed"]) _player.stats.attack_speed = s["attack_speed"].as<int>();
                if (s["defense"]) _player.stats.defense = s["defense"].as<int>();
                if (s["speed"]) _player.stats.speed = s["speed"].as<int>();
            }
        }
        
        // Parse projectiles section
        if (config["projectiles"]) {
            auto proj = config["projectiles"];
            if (proj["player"]) {
                if (proj["player"]["speed"]) _projectiles.player.speed = proj["player"]["speed"].as<float>();
                if (proj["player"]["offset_x"]) _projectiles.player.offset_x = proj["player"]["offset_x"].as<float>();
                if (proj["player"]["offset_y"]) _projectiles.player.offset_y = proj["player"]["offset_y"].as<float>();
            }
            if (proj["enemy"]) {
                if (proj["enemy"]["speed"]) _projectiles.enemy.speed = proj["enemy"]["speed"].as<float>();
                if (proj["enemy"]["offset_x"]) _projectiles.enemy.offset_x = proj["enemy"]["offset_x"].as<float>();
                if (proj["enemy"]["offset_y"]) _projectiles.enemy.offset_y = proj["enemy"]["offset_y"].as<float>();
            }
        }
        
        // Parse systems section
        if (config["systems"]) {
            auto sys = config["systems"];
            if (sys["movement"]) _systems.movement = sys["movement"].as<bool>();
            if (sys["stats"]) _systems.stats = sys["stats"].as<bool>();
            if (sys["enemy"]) _systems.enemy = sys["enemy"].as<bool>();
            if (sys["collision"]) _systems.collision = sys["collision"].as<bool>();
        }
        
        // std::cout << "Loaded game loop config from: " << filepath << std::endl;
        /*
        std::cout << "Loaded game loop config from: " << filepath << std::endl;
        std::cout << "  Tick rate: " << std::to_string(_gameLoop.tick_rate) << std::endl;
        std::cout << "  Max messages per frame: " << std::to_string(_gameLoop.max_messages_per_frame) << std::endl;
        std::cout << "  Min players to start: " << std::to_string(_gameLoop.min_players_to_start) << std::endl;
        */
        
        return true;
    } catch (const YAML::Exception& e) {
        std::cerr << "Failed to parse config file: " << e.what() << std::endl;
        loadDefaults();
        return false;
    }
}
bool GameLoopConfig::loadUpgradesFromFile(const std::string& filepath)
{
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Could not open upgrades config file: " << filepath << std::endl;
            return false;
        }

        YAML::Node config = YAML::LoadFile(filepath);
        
        if (config["upgrades"]) {
            auto upgradesNode = config["upgrades"];
            for (const auto& upgradeNode : upgradesNode) {
                UpgradeData upgrade;
                if (upgradeNode["id"]) upgrade.id = upgradeNode["id"].as<std::string>();
                if (upgradeNode["name"]) upgrade.name = upgradeNode["name"].as<std::string>();
                if (upgradeNode["description"]) upgrade.description = upgradeNode["description"].as<std::string>();
                if (upgradeNode["type"]) upgrade.type = upgradeNode["type"].as<std::string>();
                if (upgradeNode["rarity"]) upgrade.rarity = upgradeNode["rarity"].as<std::string>();
                
                // Visuals
                if (upgradeNode["visuals"]) {
                    auto vis = upgradeNode["visuals"];
                    if (vis["icon_path"]) upgrade.icon_path = vis["icon_path"].as<std::string>();
                    if (vis["card_color"]) upgrade.card_color = vis["card_color"].as<std::string>();
                }

                // Handle simple stat boost
                if (upgrade.type == "stat_boost" || upgrade.type == "add_weapon") {
                    UpgradeEffect effect;
                    if (upgradeNode["target"]) effect.target = upgradeNode["target"].as<std::string>();
                    if (upgradeNode["value"]) effect.value = upgradeNode["value"].as<float>();
                    upgrade.effects.push_back(effect);
                }
                // Handle hybrid/complex effects
                else if (upgrade.type == "hybrid" && upgradeNode["effects"]) {
                    for (const auto& effectNode : upgradeNode["effects"]) {
                        UpgradeEffect effect;
                        if (effectNode["target"]) effect.target = effectNode["target"].as<std::string>();
                        if (effectNode["value"]) effect.value = effectNode["value"].as<float>();
                        upgrade.effects.push_back(effect);
                    }
                }
                
                _upgrades.push_back(upgrade);
            }
        }
        return true;
    } catch (const YAML::Exception& e) {
        std::cerr << "Failed to parse upgrades config file: " << e.what() << std::endl;
        return false;
    }
}

bool GameLoopConfig::loadBossesFromDirectory(const std::string& dirPath)
{
    DIR* dir;
    struct dirent* ent;
    
    if ((dir = opendir(dirPath.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string filename = ent->d_name;
            if (filename.length() > 5 && filename.substr(filename.length() - 5) == ".yaml") {
                std::string filepath = dirPath + "/" + filename;
                try {
                    YAML::Node config = YAML::LoadFile(filepath);
                    if (config["enemies"]) {
                        auto enemies = config["enemies"];
                        for (const auto& enemy : enemies) {
                            if (enemy["type"] && enemy["type"].as<std::string>() == "boss") {
                                BossConfig boss;
                                if (enemy["id"]) boss.id = enemy["id"].as<std::string>();
                                if (enemy["name"]) boss.name = enemy["name"].as<std::string>();
                                if (enemy["stats"]) {
                                    if (enemy["stats"]["health"]) boss.health = enemy["stats"]["health"].as<int>();
                                    if (enemy["stats"]["max_health"]) boss.max_health = enemy["stats"]["max_health"].as<int>();
                                    if (enemy["stats"]["speed"]) boss.speed = enemy["stats"]["speed"].as<float>();
                                }
                                
                                if (enemy["visuals"]) {
                                    auto visuals = enemy["visuals"];
                                    if (visuals["texture_path"]) boss.visuals.texture_path = visuals["texture_path"].as<std::string>();
                                    if (visuals["scale"]) boss.visuals.scale = visuals["scale"].as<float>();
                                    if (visuals["offset_x"]) boss.visuals.offset_x = visuals["offset_x"].as<float>();
                                    if (visuals["offset_y"]) boss.visuals.offset_y = visuals["offset_y"].as<float>();

                                    // Explicit visual dimensions
                                    if (visuals["width"]) boss.visuals.width = visuals["width"].as<float>();
                                    if (visuals["height"]) boss.visuals.height = visuals["height"].as<float>();

                                    if (visuals["collider_box"]) {
                                        auto collider = visuals["collider_box"];
                                        // Load collider dimensions
                                        if (collider["width"]) boss.visuals.collider_width = collider["width"].as<float>();
                                        if (collider["height"]) boss.visuals.collider_height = collider["height"].as<float>();
                                        
                                        // Fallback: Use collider dimensions if visual dimensions are missing
                                        if (!visuals["width"] && collider["width"]) boss.visuals.width = boss.visuals.collider_width;
                                        if (!visuals["height"] && collider["height"]) boss.visuals.height = boss.visuals.collider_height;
                                    }
                                    
                                    if (visuals["animations"]) {
                                        auto anims = visuals["animations"];
                                        for (YAML::const_iterator it = anims.begin(); it != anims.end(); ++it) {
                                            std::string key = it->first.as<std::string>();
                                            YAML::Node animNode = it->second;
                                            BossAnimation anim;
                                            if (animNode["start_x"]) anim.start_x = animNode["start_x"].as<int>();
                                            if (animNode["start_y"]) anim.start_y = animNode["start_y"].as<int>();
                                            if (animNode["width"]) anim.width = animNode["width"].as<int>();
                                            if (animNode["height"]) anim.height = animNode["height"].as<int>();
                                            if (animNode["frame_count"]) anim.frame_count = animNode["frame_count"].as<int>();
                                            if (animNode["frame_duration"]) anim.frame_duration = animNode["frame_duration"].as<float>();
                                            if (animNode["loop"]) anim.loop = animNode["loop"].as<bool>();
                                            boss.visuals.animations[key] = anim;
                                        }
                                    }
                                }
                                
                                if (enemy["behavior"]) {
                                    auto behaviors = enemy["behavior"];
                                    if (behaviors["phases"]) {
                                        for (const auto& phaseNode : behaviors["phases"]) {
                                            BossPhase phase;
                                            if (phaseNode["id"]) phase.id = phaseNode["id"].as<int>();
                                            if (phaseNode["name"]) phase.name = phaseNode["name"].as<std::string>();
                                            if (phaseNode["trigger_health_percentage"]) phase.trigger_health_percentage = phaseNode["trigger_health_percentage"].as<int>();
                                            if (phaseNode["visual_effect"]) phase.visual_effect = phaseNode["visual_effect"].as<std::string>();
                                            if (phaseNode["loop_patterns"]) phase.loop_patterns = phaseNode["loop_patterns"].as<bool>();
                                            
                                            if (phaseNode["patterns"]) {
                                                for (const auto& pat : phaseNode["patterns"]) {
                                                    phase.patterns.push_back(pat.as<std::string>());
                                                }
                                            }
                                            boss.phases.push_back(phase);
                                        }
                                    }
                                    if (behaviors["attack_patterns"]) {
                                        for (const auto& patNode : behaviors["attack_patterns"]) {
                                            BossAttackPattern pattern;
                                            if (patNode["id"]) pattern.id = patNode["id"].as<std::string>();
                                            if (patNode["type"]) pattern.type = patNode["type"].as<std::string>();
                                            if (patNode["projectile_id"]) pattern.projectile_id = patNode["projectile_id"].as<std::string>();
                                            if (patNode["count"]) pattern.count = patNode["count"].as<int>();
                                            if (patNode["duration"]) pattern.duration = patNode["duration"].as<float>();
                                            if (patNode["damage"]) pattern.damage = patNode["damage"].as<float>();
                                            if (patNode["speed"]) pattern.speed = patNode["speed"].as<float>();
                                            if (patNode["warning_time"]) pattern.warning_time = patNode["warning_time"].as<float>();
                                            if (patNode["animation_trigger"]) pattern.animation_trigger = patNode["animation_trigger"].as<std::string>();
                                            
                                            boss.patterns.push_back(pattern);
                                        }
                                    }
                                }
                                
                                _bosses[boss.id] = boss;
                                std::cout << "Loaded boss: " << boss.name << " (" << boss.id << ")" << std::endl;
                            }
                        }
                    }
                } catch (const YAML::Exception& e) {
                    std::cerr << "Failed to parse boss config file: " << filepath << " " << e.what() << std::endl;
                }
            }
        }
        closedir(dir);
        return true;
    } else {
        std::cerr << "Could not open directory: " << dirPath << std::endl;
        return false;
    }
}
