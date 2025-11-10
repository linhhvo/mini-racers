#pragma once

#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <array>

class Track : public sf::Drawable {
public:
    Track();
    ~Track() = default;
    
    bool loadFromFile(const std::string& filepath);
    void update(float dt);
    
    sf::Vector2u getMapSize() const { return mapSize_; }
    sf::Vector2u getTileSize() const { return tileSize_; }
    sf::Color getBackgroundColor() const;

    struct SpawnPoint {
        sf::Vector2f position; // top-left position in pixels
        sf::Vector2f size;     // width/height in pixels
        float rotation = 0.f;  // rotation in degrees
        std::string textureId; // identifier or filename for texture
        std::uint32_t tileID = 0; // GID if object uses a tile from tileset
        std::string name;
        std::string layerName;
    };

    const std::vector<SpawnPoint>& getVehicleSpawnPoints() const { return vehicleSpawns_; }
    
    // Extract a tile texture from the tileset by GID
    std::unique_ptr<sf::Texture> extractTileTexture(std::uint32_t tileGID) const;
    
    // Checkpoint zone structure
    struct CheckpointZone {
        int id;                    // Unique checkpoint ID
        sf::Vector2f position;     // Top-left position in pixels
        sf::Vector2f size;         // Width/height in pixels
        bool activated = false;    // Whether this checkpoint has been passed
        float activationTimer = 0.0f; // Timer for how long checkpoint has been activated (in seconds)
        static constexpr float ACTIVATION_DURATION = 0.5f; // Stay active for 0.5 seconds after leaving
    };
    
    const std::vector<CheckpointZone>& getCheckpointZones() const { return checkpointZones_; }
    
    // Check if a point (in pixels) is inside a checkpoint zone and activate it
    // Also updates activation timers and deactivates checkpoints when car leaves
    // Returns the checkpoint ID if newly activated, or -1 if none
    int checkAndActivateCheckpoint(const sf::Vector2f& position);
    
    // Update checkpoint timers (call this in update())
    void updateCheckpoints(float dt);
    
    // Get activated checkpoint IDs
    std::vector<int> getActivatedCheckpoints() const;
    
private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    void createLayer(const tmx::Map& map, const tmx::TileLayer& layer, const std::string& layerName);
    void loadTextures(const tmx::Map& map);
    void parseObjectLayers(const tmx::Map& map);
    
    tmx::Map map_;
    bool mapLoaded_;
    
    sf::Vector2u mapSize_;
    sf::Vector2u tileSize_;
    
    struct TileQuad {
        std::array<sf::Vertex, 4> vertices;
        const sf::Texture* texture;
    };
    
    // Store tiles per layer name for show/hide functionality
    std::map<std::string, std::vector<TileQuad>> layerTiles_;
    std::vector<std::string> layerOrder_; // Store layer names in the order they appear in TMX (for correct rendering order)
    std::vector<TileQuad> tiles_; // Keep for backward compatibility, contains all tiles
    std::map<std::string, std::unique_ptr<sf::Texture>> textures_;
    std::vector<SpawnPoint> vehicleSpawns_;
    std::vector<CheckpointZone> checkpointZones_;
};

