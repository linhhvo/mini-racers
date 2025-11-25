#include "System/AudioManager.hpp"
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>

// Suppress ALSA/OpenAL error messages (redirects stderr to /dev/null)
static void suppressAudioErrors() {
    int devnull = open("/dev/null", O_WRONLY);
    if (devnull != -1) {
        dup2(devnull, STDERR_FILENO);
        close(devnull);
    }
}

// Constructor - suppress audio device errors on initialization
AudioManager::AudioManager() {
    suppressAudioErrors();
    // Initialize default volumes
}

// Static sound effect object - keeps sound alive while playing
sf::Music AudioManager::soundEffect_;

// Get or create singleton instance
AudioManager& AudioManager::Instance() {
    static AudioManager instance;
    return instance;
}

/// Play sound directly from file
void AudioManager::PlaySound(const std::string& filename) {
    if (soundEffect_.openFromFile(filename)) {
        soundEffect_.setVolume(soundVolume_);
        soundEffect_.play();
        std::cout << "[Audio] Playing sound: " << filename << std::endl;
    } else {
        std::cerr << "[Audio] ERROR: Failed to play: " << filename << std::endl;
    }
}

/// Play background music with looping
void AudioManager::PlayMusic(const std::string& filename) {
    if (music_.openFromFile(filename)) {
        music_.setLoop(true);
        music_.setVolume(musicVolume_);
        music_.play();
        std::cout << "[Audio] Playing music: " << filename << std::endl;
    } else {
        std::cerr << "[Audio] ERROR: Failed to play music: " << filename << std::endl;
    }
}

/// Stop background music playback
void AudioManager::StopMusic() {
    std::cout << "[Audio] Stopping music" << std::endl;
    music_.stop();
}

/// Get current master volume
float AudioManager::GetVolume() const {
    return soundVolume_;
}

/// Set music volume (0.0 - 100.0)
void AudioManager::SetMusicVolume(float volume) {
    musicVolume_ = (volume < 0.0f) ? 0.0f : (volume > 100.0f) ? 100.0f : volume;
    music_.setVolume(musicVolume_);
    std::cout << "[Audio] Music volume set to " << musicVolume_ << "%" << std::endl;
}

/// Set sound effect volume (0.0 - 100.0)
void AudioManager::SetSoundVolume(float volume) {
    soundVolume_ = (volume < 0.0f) ? 0.0f : (volume > 100.0f) ? 100.0f : volume;
    soundEffect_.setVolume(soundVolume_);
    std::cout << "[Audio] Sound volume set to " << soundVolume_ << "%" << std::endl;
}
