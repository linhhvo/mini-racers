#pragma once
#include <SFML/Audio.hpp>
#include <string>
#include <map>
#include <vector>

/**
 * @class AudioManager
 * @brief Singleton audio manager for game sound and music playback
 *
 * Provides a centralized interface for managing game audio:
 * - Background music playback (looping)
 * - Sound effects playback
 * - Error suppression for audio devices on headless systems
 */
class AudioManager {
public:
    /// Get singleton instance
    static AudioManager& Instance();

    /// Load sound into buffer for reuse (unused)
    bool LoadSound(const std::string& name, const std::string& filename);

    /// Play pre-loaded sound by name (unused)
    void play(const std::string& name);

    /// Play sound directly from file (recommended)
    void PlaySound(const std::string& filename);

    /// Pre-load music from file (unused)
    void LoadMusic(const std::string& filePath);

    /// Play music with looping enabled
    void PlayMusic(const std::string& filename);

    /// Stop a named sound (unused)
    void stop(const std::string& name);

    /// Stop current background music
    void StopMusic();

    /// Stop all sounds and music
    void StopAll();

    /// Update - clean up stopped sounds
    void Update();

    /// Set master volume for all audio (0.0 - 100.0)
    void SetVolume(float volume);

    /// Get current master volume (0.0 - 100.0)
    float GetVolume() const;

private:
    AudioManager();
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    sf::Music music_;                                     ///< Background music
    static sf::Music soundEffect_;                        ///< Sound effect (kept alive)
    std::map<std::string, sf::SoundBuffer> soundBuffers_; ///< Pre-loaded buffers (unused)
    std::vector<std::pair<std::string, sf::Sound>> activeSounds_; ///< Active sounds (unused)
    float volume_ = 100.0f;                               ///< Master volume (0.0 - 100.0)
};
