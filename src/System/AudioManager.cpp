#include "System/AudioManager.hpp"
#include <SFML/Audio.hpp>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

AudioManager::AudioManager() {}

AudioManager& AudioManager::Instance() {
    static AudioManager instance;
    return instance;
}


bool AudioManager::LoadSound(const std::string& name, const std::string& filename) {
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(filename))
        return false;
    buffers_[name] = buffer;
    return true;
}

void AudioManager::PlaySound(const std::string& name) { //no volume control yet
    auto it = buffers_.find(name);
    if (it != buffers_.end()) {
        sf::Sound sound;
        sound.setBuffer(it->second);
        //sound.setVolume(volume);
        sound.play();
        // store the active sound (name + instance) so overlapping playback works
        sounds_.emplace_back(name, sound);
        std::cout << "[Audio] PlaySound called for '" << name << "'\n";
    }
}

void AudioManager::Update() {
    // Remove stopped sounds from the map
    // remove stopped sounds and log which named instance stopped
    auto newEnd = std::remove_if(sounds_.begin(), sounds_.end(), [](const std::pair<std::string, sf::Sound>& kv) {
        if (kv.second.getStatus() == sf::Sound::Stopped) {
            std::cout << "[Audio] Sound stopped: '" << kv.first << "'\n";
            return true;
        }
        return false;
    });
    sounds_.erase(newEnd, sounds_.end());
}

void AudioManager::LoadMusic(const std::string& filePath) {
    // Attempt to open the provided file into the single music_ member
    music_.openFromFile(filePath);
}

void AudioManager::PlayMusic(bool loop) {
    music_.setLoop(loop);
    music_.play();
}

void AudioManager::StopMusic() {
    music_.stop();
}

void AudioManager::StopAll() {
    for (auto& kv : sounds_) {
        kv.second.stop();
    }
    sounds_.clear();

    // stop single music member (if used)
    music_.stop();
}
