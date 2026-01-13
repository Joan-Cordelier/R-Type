#include "AudioManager.hpp"

AudioManager::AudioManager() : _currentMusic(nullptr)
{
    // Initialize SDL_mixer for MP3 and OGG support
    int flags = MIX_INIT_MP3 | MIX_INIT_OGG;
    if (Mix_Init(flags) != flags) {
        std::cerr << "SDL_mixer could not initialize support for MP3/OGG: " << Mix_GetError() << std::endl;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
    }
}

AudioManager::~AudioManager()
{
    for (auto& pair : _musicLibrary) {
        Mix_FreeMusic(pair.second);
    }
    _musicLibrary.clear();

    for (auto& pair : _soundLibrary) {
        Mix_FreeChunk(pair.second);
    }
    _soundLibrary.clear();

    Mix_Quit();
}

void AudioManager::playMusic(const std::string& path, bool loop)
{
    if (_musicLibrary.find(path) == _musicLibrary.end()) {
        Mix_Music* music = Mix_LoadMUS(path.c_str());
        if (music == nullptr) {
            std::cerr << "Failed to load music! SDL_mixer Error: " << Mix_GetError() << std::endl;
            return;
        }
        _musicLibrary[path] = music;
    }

    if (_currentMusic != _musicLibrary[path]) {
        _currentMusic = _musicLibrary[path];
        Mix_PlayMusic(_currentMusic, loop ? -1 : 0);
    } else {
        if (Mix_PlayingMusic() == 0) {
            Mix_PlayMusic(_currentMusic, loop ? -1 : 0);
        }
    }
}

void AudioManager::playSound(const std::string& path)
{
    if (_soundLibrary.find(path) == _soundLibrary.end()) {
        Mix_Chunk* sound = Mix_LoadWAV(path.c_str());
        if (sound == nullptr) {
            std::cerr << "Failed to load sound effect! SDL_mixer Error: " << Mix_GetError() << std::endl;
            return;
        }
        _soundLibrary[path] = sound;
    }
    Mix_PlayChannel(-1, _soundLibrary[path], 0);
}

void AudioManager::setMusicVolume(int volume)
{
    if (volume < 0) volume = 0;
    if (volume > 128) volume = 128; // specific to SDL mixer
    Mix_VolumeMusic(volume);
}

void AudioManager::setSoundVolume(int volume)
{
    if (volume < 0) volume = 0;
    if (volume > 128) volume = 128;
    Mix_Volume(-1, volume); // -1 for all channels
}

void AudioManager::stopMusic()
{
    Mix_HaltMusic();
}