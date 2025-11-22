#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/System/Vector2.hpp>
#include <array>
#include <map>
#include <memory>
#include <string>
#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
#include <vector>

class CheckpointManager;

class Track : public sf::Drawable {
public:
  Track();
  ~Track() = default;

  bool loadFromFile(const std::string &filepath);

  sf::Vector2u getMapSize() const { return mapSize_; }
  sf::Vector2u getTileSize() const { return tileSize_; }
  sf::Color getBackgroundColor() const;
  const tmx::Map &getTmxMap() const { return map_; }

  struct SpawnPoint {
    sf::Vector2f position;    // top-left position in pixels
    sf::Vector2f size;        // width/height in pixels
    float rotation = 0.f;     // rotation in degrees
    std::string textureId;    // identifier or filename for texture
    std::uint32_t tileID = 0; // GID if object uses a tile from tileset
    std::string name;
    std::string layerName;
  };

  const std::vector<SpawnPoint> &getVehicleSpawnPoints() const {
    return vehicleSpawns_;
  }

  // Extract a tile texture from the tileset by GID
  std::unique_ptr<sf::Texture> extractTileTexture(std::uint32_t tileGID) const;

  void setCheckpointManager(CheckpointManager *checkpointManager) {
    checkpointManager_ = checkpointManager;
  }

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
  void createLayer(const tmx::Map &map, const tmx::TileLayer &layer,
                   const std::string &layerName);
  void loadTextures(const tmx::Map &map);
  void parseObjectLayers(const tmx::Map &map);

  tmx::Map map_;
  bool mapLoaded_;

  sf::Vector2u mapSize_;
  sf::Vector2u tileSize_;

  struct TileQuad {
    std::array<sf::Vertex, 4> vertices;
    const sf::Texture *texture;
  };

  // Store tiles per layer name for show/hide functionality
  std::map<std::string, std::vector<TileQuad>> layerTiles_;

  // Store layer names in the order they appear in TMX (for correct rendering
  // order)
  std::vector<std::string> layerOrder_;

  // Keep for backward compatibility, contains all tiles
  std::vector<TileQuad> tiles_;

  std::map<std::string, std::unique_ptr<sf::Texture>> textures_;
  std::vector<SpawnPoint> vehicleSpawns_;

  CheckpointManager *checkpointManager_ = nullptr;
};
