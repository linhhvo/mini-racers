#include "System/AudioManager.hpp"
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>

/// Suppress ALSA/OpenAL error messages (redirects stderr to /dev/null)
static void suppressAudioErrors() {
    int devnull = open("/dev/null", O_WRONLY);
    if (devnull != -1) {
        dup2(devnull, STDERR_FILENO);
        close(devnull);
    }
}

/// Static sound effect object - keeps sound alive while playing
sf::Music AudioManager::soundEffect_;

/// Constructor - suppress audio device errors on initialization
AudioManager::AudioManager() {
    suppressAudioErrors();
    // Set initial volumes
    SetVolume(70.0f);  // Default master volume at 70%
}

/// Get or create singleton instance
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
    std::cout << "[Audio] SUCCESS: Loaded sound '" << name << "'" << std::endl;
    return true;
}

/// Play sound directly from file (recommended)
void AudioManager::PlaySound(const std::string& filename) {
    if (soundEffect_.openFromFile(filename)) {
        soundEffect_.setVolume(volume_);
        soundEffect_.play();
        std::cout << "[Audio] Playing sound: " << filename << std::endl;
    } else {
        std::cerr << "[Audio] ERROR: Failed to play: " << filename << std::endl;
    }
}

/// Play pre-loaded sound by name (unused)
void AudioManager::play(const std::string& name) {
    auto it = soundBuffers_.find(name);
    if (it != soundBuffers_.end()) {
        sf::Sound sound;
        sound.setBuffer(it->second);
        sound.setVolume(volume_);
        sound.play();
        activeSounds_.emplace_back(name, sound);
        std::cout << "[Audio] Playing named sound: " << name << std::endl;
    } else {
        std::cerr << "[Audio] ERROR: Sound not found: " << name << std::endl;
    }
}

/// Play background music with looping
void AudioManager::PlayMusic(const std::string& filename) {
    if (music_.openFromFile(filename)) {
        music_.setLoop(true);
        music_.setVolume(volume_);
        music_.play();
        std::cout << "[Audio] Playing music: " << filename << std::endl;
    } else {
        std::cerr << "[Audio] ERROR: Failed to play music: " << filename << std::endl;
    }
}

/// Pre-load music from file (unused)
void AudioManager::LoadMusic(const std::string& filePath) {
    std::cout << "[Audio] LoadMusic: Loading from '" << filePath << "'" << std::endl;
    if (!music_.openFromFile(filePath)) {
        std::cerr << "[Audio] ERROR: Failed to load music: " << filePath << std::endl;
    } else {
        std::cout << "[Audio] SUCCESS: Music loaded" << std::endl;
    }
}

/// Stop a named sound (unused)
void AudioManager::stop(const std::string& name) {
    std::cout << "[Audio] Stopping sound '" << name << "'" << std::endl;
    int stopped = 0;
    for (auto& kv : activeSounds_) {
        if (kv.first == name) {
            kv.second.stop();
            stopped++;
        }
    }
}

/// Stop background music playback
void AudioManager::StopMusic() {
    std::cout << "[Audio] Stopping music" << std::endl;
    music_.stop();
}

/// Stop all audio (sounds and music)
void AudioManager::StopAll() {
    std::cout << "[Audio] Stopping all audio" << std::endl;
    music_.stop();
    soundEffect_.stop();
    for (auto& kv : activeSounds_) {
        kv.second.stop();
    }
    activeSounds_.clear();
}

/// Update - remove stopped sounds from active list
void AudioManager::Update() {
    auto newEnd = std::remove_if(activeSounds_.begin(), activeSounds_.end(),
        [](const std::pair<std::string, sf::Sound>& kv) {
            return kv.second.getStatus() == sf::Sound::Stopped;
        });
    activeSounds_.erase(newEnd, activeSounds_.end());
}

/// Set master volume for all audio (0.0 - 100.0)
void AudioManager::SetVolume(float volume) {
    // Clamp volume to 0.0 - 100.0
    volume_ = (volume < 0.0f) ? 0.0f : (volume > 100.0f) ? 100.0f : volume;
    
    // Apply to music and sound effect
    music_.setVolume(volume_);
    soundEffect_.setVolume(volume_);
    
    // Apply to all active sounds
    for (auto& kv : activeSounds_) {
        kv.second.setVolume(volume_);
    }
    
    std::cout << "[Audio] Volume set to " << volume_ << "%" << std::endl;
}

/// Get current master volume
float AudioManager::GetVolume() const {
    return volume_;
}
