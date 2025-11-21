#include "System/AudioManager.hpp"
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>

// Suppress ALSA/OpenAL errors
static void suppressAudioErrors() {
    int devnull = open("/dev/null", O_WRONLY);
    if (devnull != -1) {
        dup2(devnull, STDERR_FILENO);
        close(devnull);
    }
}

// Static sound effect - keeps sound alive while playing
sf::Music AudioManager::soundEffect_;

AudioManager::AudioManager() {
    suppressAudioErrors();
}

AudioManager& AudioManager::Instance() {
    static AudioManager instance;
    return instance;
}

bool AudioManager::LoadSound(const std::string& name, const std::string& filename) {
    std::cout << "[Audio] LoadSound: Loading '" << name << "' from '" << filename << "'" << std::endl;
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(filename)) {
        std::cerr << "[Audio] ERROR: Failed to load sound: " << filename << std::endl;
        return false;
    }
    soundBuffers_[name] = buffer;
    std::cout << "[Audio] SUCCESS: Loaded sound '" << name << "' (" << buffer.getSampleCount() 
              << " samples, " << buffer.getChannelCount() << " channels)" << std::endl;
    return true;
}

void AudioManager::PlaySound(const std::string& filename) {
    // Direct file playback - simple approach
    if (soundEffect_.openFromFile(filename)) {
        soundEffect_.play();
        std::cout << "[Audio] SUCCESS: Playing sound: " << filename 
                  << " (Duration: " << soundEffect_.getDuration().asSeconds() << "s)" << std::endl;
    } else {
        std::cerr << "[Audio] ERROR: Failed to play: " << filename << std::endl;
    }
}

void AudioManager::play(const std::string& name) {
    auto it = soundBuffers_.find(name);
    if (it != soundBuffers_.end()) {
        sf::Sound sound;
        sound.setBuffer(it->second);
        sound.play();
        activeSounds_.emplace_back(name, sound);
        std::cout << "[Audio] SUCCESS: Playing named sound: " << name 
                  << " (Active sounds: " << activeSounds_.size() << ")" << std::endl;
    } else {
        std::cerr << "[Audio] ERROR: Sound not found: " << name << std::endl;
    }
}

void AudioManager::PlayMusic(const std::string& filename) {
    if (music_.openFromFile(filename)) {
        music_.setLoop(true);
        music_.play();
        std::cout << "[Audio] SUCCESS: Playing music: " << filename 
                  << " (Duration: " << music_.getDuration().asSeconds() << "s)" << std::endl;
    } else {
        std::cerr << "[Audio] ERROR: Failed to play music: " << filename << std::endl;
    }
}

void AudioManager::LoadMusic(const std::string& filePath) {
    std::cout << "[Audio] LoadMusic: Loading from '" << filePath << "'" << std::endl;
    if (!music_.openFromFile(filePath)) {
        std::cerr << "[Audio] ERROR: Failed to load music: " << filePath << std::endl;
    } else {
        std::cout << "[Audio] SUCCESS: Music loaded (Duration: " << music_.getDuration().asSeconds() << "s)" << std::endl;
    }
}

void AudioManager::stop(const std::string& name) {
    std::cout << "[Audio] stop: Stopping sound '" << name << "'" << std::endl;
    int stopped = 0;
    for (auto& kv : activeSounds_) {
        if (kv.first == name) {
            kv.second.stop();
            stopped++;
        }
    }
    std::cout << "[Audio] Stopped " << stopped << " instance(s) of '" << name << "'" << std::endl;
}

void AudioManager::StopMusic() {
    std::cout << "[Audio] StopMusic: Stopping background music" << std::endl;
    music_.stop();
}

void AudioManager::StopAll() {
    std::cout << "[Audio] StopAll: Stopping all sounds and music" << std::endl;
    music_.stop();
    soundEffect_.stop();
    for (auto& kv : activeSounds_) {
        kv.second.stop();
    }
    std::cout << "[Audio] Cleared " << activeSounds_.size() << " active sounds" << std::endl;
    activeSounds_.clear();
}

void AudioManager::Update() {
    // Clean up stopped sounds
    auto oldSize = activeSounds_.size();
    auto newEnd = std::remove_if(activeSounds_.begin(), activeSounds_.end(),
        [](const std::pair<std::string, sf::Sound>& kv) {
            return kv.second.getStatus() == sf::Sound::Stopped;
        });
    int removed = activeSounds_.end() - newEnd;
    activeSounds_.erase(newEnd, activeSounds_.end());
    if (removed > 0) {
        std::cout << "[Audio] Update: Cleaned up " << removed << " stopped sound(s) (" 
                  << oldSize << " -> " << activeSounds_.size() << ")" << std::endl;
    }
}
