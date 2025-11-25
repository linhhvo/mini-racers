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

    /// Play sound directly from file
    void PlaySound(const std::string& filename);

    /// Play music with looping enabled
    void PlayMusic(const std::string& filename);

    /// Stop current background music
    void StopMusic();

    /// Get current master volume (0.0 - 100.0)
    float GetVolume() const;

    /// Set music volume (0.0 - 100.0)
    void SetMusicVolume(float volume);

    /// Set sound effect volume (0.0 - 100.0)
    void SetSoundVolume(float volume);

private:
    AudioManager();
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    sf::Music music_;                                     ///< Background music
    static sf::Music soundEffect_;                        ///< Sound effect (kept alive)
    float musicVolume_ = 70.0f;                           ///< Music volume (0.0 - 100.0)
    float soundVolume_ = 70.0f;                           ///< Sound effect volume (0.0 - 100.0)
};
