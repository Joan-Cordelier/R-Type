#pragma once

#include <SDL2/SDL_mixer.h>
#include <string>
#include <map>
#include <iostream>

class AudioManager {
    public:
        AudioManager();
        ~AudioManager();

        void playMusic(const std::string& path, bool loop = true);
        void playSound(const std::string& path);
        void setMusicVolume(int volume);
        void setSoundVolume(int volume);
        void stopMusic();

    private:
        std::map<std::string, Mix_Music*> _musicLibrary;
        std::map<std::string, Mix_Chunk*> _soundLibrary;
        Mix_Music* _currentMusic;
};