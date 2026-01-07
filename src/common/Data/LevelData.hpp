#pragma once
#include <string>
#include <vector>
#include <map>

struct EnemyTypeData {
    std::string name;
    std::string sprite_path;
    std::string texture_id;
    int health = 50;
    int damage = 10;
    int speed = 100;
    int score = 100;
    float width = 50.0f;
    float height = 50.0f;
    float velocity_x = -100.0f;
    float velocity_y = 0.0f;
    float move_amplitude = 0.0f;
    float move_frequency = 0.0f;
};

struct EnemyWaveGroupData {
    std::string type;
    int count = 1;
    float interval = 1.0f;
};

struct WaveData {
    int id;
    float start_delay = 0.0f;
    std::vector<EnemyWaveGroupData> groups;
};

struct LevelData {
    int id;
    std::string name;
    std::string background_path;
    std::string music_path;
    std::vector<WaveData> waves;
};
