#pragma once
#include <SFML/Audio.hpp>
#include <string>
#include <map>
#include <vector>

class AudioManager {
public:
    static AudioManager& Instance();

    // Load sound into buffer (for reuse)
    bool LoadSound(const std::string& name, const std::string& filename);

    // Play sound by name (must be pre-loaded)
    void play(const std::string& name);

    // Play sound directly from file
    void PlaySound(const std::string& filename);

    // Load music from file
    void LoadMusic(const std::string& filePath);

    // Play the loaded music
    void PlayMusic(const std::string& filename);

    // Stop a named sound
    void stop(const std::string& name);

    // Stop current music
    void StopMusic();

    // Stop all sounds and music
    void StopAll();

    // Update (clean up stopped sounds)
    void Update();

private:
    AudioManager();
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    sf::Music music_;                                    // Current looping music
    static sf::Music soundEffect_;                       // Sound effect (kept alive)
    std::map<std::string, sf::SoundBuffer> soundBuffers_; // Pre-loaded buffers
    std::vector<std::pair<std::string, sf::Sound>> activeSounds_; // Active sounds
};
