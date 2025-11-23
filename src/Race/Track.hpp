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
#include <set>
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

  void setActiveCheckpointLayers(const std::set<std::string> &layers) {
    activeCheckpointLayers_ = layers;
  }
  // Extract a tile texture from the tileset by GID
  std::unique_ptr<sf::Texture> extractTileTexture(std::uint32_t tileGID) const;

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
  void createLayer(const tmx::Map &map, const tmx::TileLayer &layer,
                   const std::string &layerName);
  void loadTextures(const tmx::Map &map);

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

  std::set<std::string> activeCheckpointLayers_;
};
