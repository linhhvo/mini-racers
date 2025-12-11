#include "System/AudioManager.hpp"
#include <fcntl.h>
#include <iostream>
#include <unistd.h>


// Constructor
AudioManager::AudioManager() {}

// Static sound effect object - keeps sound alive while playing
sf::Music AudioManager::soundEffect_;

// Get or create singleton instance
AudioManager &AudioManager::Instance() {
  static AudioManager instance;
  return instance;
}

/// Play sound directly from file
void AudioManager::PlaySound(const std::string &filename) {
  if (soundEffect_.openFromFile(filename)) {
    soundEffect_.setVolume(soundVolume_);
    soundEffect_.play();
  } else {
    std::cerr << "[Audio] ERROR: Failed to play: " << filename << std::endl;
  }
}

/// Play background music with looping
void AudioManager::PlayMusic(const std::string &filename) {
  if (music_.openFromFile(filename)) {
    music_.setLoop(true);
    music_.setVolume(musicVolume_);
    music_.play();
  } else {
    std::cerr << "[Audio] ERROR: Failed to play music: " << filename
              << std::endl;
  }
}

/// Stop background music playback
void AudioManager::StopMusic() { music_.stop(); }

/// Get current master volume
float AudioManager::GetVolume() const { return soundVolume_; }

/// Set music volume (0.0 - 100.0)
void AudioManager::SetMusicVolume(float volume) {
  musicVolume_ = (volume < 0.0f) ? 0.0f : (volume > 100.0f) ? 100.0f : volume;
  music_.setVolume(musicVolume_);
}

/// Set sound effect volume (0.0 - 100.0)
void AudioManager::SetSoundVolume(float volume) {
  soundVolume_ = (volume < 0.0f) ? 0.0f : (volume > 100.0f) ? 100.0f : volume;
  soundEffect_.setVolume(soundVolume_);
}
