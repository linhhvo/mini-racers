#pragma once
#include <SFML/Audio.hpp>
#include <string>
#include <map>

class AudioManager {
public:
    static AudioManager& Instance();

    // Play short sound effect
    void PlaySound(const std::string& filename);

    // Play looping music
    void PlayMusic(const std::string& filename);

    // Stop current music
    void StopMusic();

    // Stop all sounds and music
    void StopAll();

    // Optional: load music once if you want
    void loadMusic(const std::string& name, const std::string& filepath);
    void play(const std::string& name);
    void stop(const std::string& name);

private:
    AudioManager();
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    static void suppressAudioErrors();

    sf::Music music_;             // current looping music
    static sf::Music soundEffect; // sound effect, kept alive
    std::map<std::string, sf::Music*> musics_; // optional multiple music
    sf::Music* currentMusic_ = nullptr;
};
