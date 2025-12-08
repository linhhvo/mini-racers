#include "Track.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <tmxlite/Object.hpp>
#include <tmxlite/ObjectGroup.hpp>
#include <tmxlite/Property.hpp>
#include <tmxlite/Tileset.hpp>

Track::Track() : mapLoaded_(false), mapSize_(0, 0), tileSize_(0, 0) {}

bool Track::loadFromFile(const std::string &filepath) {
  if (!map_.load(filepath)) {
    // std::cout << "Failed to load TMX map: " << filepath << std::endl;
    return false;
  }

  if (map_.getOrientation() != tmx::Orientation::Orthogonal) {
    // std::cout << "Map is not orthogonal - only orthogonal maps are supported"
    //           << std::endl;
    return false;
  }

  // Check if tilesets loaded properly
  const auto &tilesets = map_.getTilesets();
  if (tilesets.empty()) {
    // std::cout << "Error: No tilesets found in map. Make sure tilesets are "
    //              "embedded in the TMX file."
    //           << std::endl;
    return false;
  }

  bool hasValidTileset = false;
  for (const auto &tileset : tilesets) {
    if (!tileset.getImagePath().empty()) {
      hasValidTileset = true;
      break;
    }
  }

  if (!hasValidTileset) {
    // std::cout << "Error: No valid tilesets with images found." << std::endl;
    return false;
  }

  mapLoaded_ = true;

  // Get map dimensions
  auto bounds = map_.getBounds();
  mapSize_.x = static_cast<unsigned int>(bounds.width);
  mapSize_.y = static_cast<unsigned int>(bounds.height);

  auto ts = map_.getTileSize();
  tileSize_.x = ts.x;
  tileSize_.y = ts.y;

  // Load textures
  loadTextures(map_);

  // Create layers
  tiles_.clear();
  layerTiles_.clear();
  layerOrder_.clear();

  const auto &layers = map_.getLayers();

  for (const auto &layer : layers) {
    if (layer->getType() == tmx::Layer::Type::Tile) {
      std::string layerName = layer->getName();
      createLayer(map_, layer->getLayerAs<tmx::TileLayer>(), layerName);
      // Store layer order to preserve TMX rendering order (bottom to top)
      layerOrder_.push_back(layerName);
      std::cout << layerName << std::endl;
    }
  }

  return true;
}

void Track::loadTextures(const tmx::Map &map) {
  textures_.clear();

  const auto &tilesets = map.getTilesets();
  for (const auto &tileset : tilesets) {
    const std::string &imagePath = tileset.getImagePath();

    if (imagePath.empty()) {
      // std::cout << "Warning: Tileset '" << tileset.getName()
      //           << "' has no image path" << std::endl;
      // std::cout << "  This usually means the tileset is external (.tsx file)
      // "
      //              "and wasn't found."
      //           << std::endl;
      // std::cout << "  Solution: In Tiled, right-click the tileset -> 'Embed "
      //              "Tileset' to embed it in the TMX file."
      //           << std::endl;
      continue;
    }

    auto texture = std::make_unique<sf::Texture>();
    sf::Image img;

    // Try to load the image - path might be relative to TMX file location
    std::string fullPath = imagePath;
    if (!img.loadFromFile(fullPath)) {
      // Try relative to assets/maps directory
      fullPath = "../assets/maps/" + imagePath;
      if (!img.loadFromFile(fullPath)) {
        // std::cout << "Warning: Failed to load tileset image: " << imagePath
        //           << std::endl;
        // std::cout << "  Tried: " << imagePath << " and " << fullPath
        //           << std::endl;
        // Create a fallback magenta texture
        img.create(32, 32, sf::Color::Magenta);
      }
    }

    if (img.getSize().x > 0 && img.getSize().y > 0) {
      // Handle transparency
      if (tileset.hasTransparency()) {
        auto transparency = tileset.getTransparencyColour();
        img.createMaskFromColor(
            {transparency.r, transparency.g, transparency.b, transparency.a});
      }
      texture->loadFromImage(img);
      textures_[imagePath] = std::move(texture);
      // std::cout << "Loaded tileset image: " << imagePath << std::endl;
    }
  }
}

void Track::createLayer(const tmx::Map &map, const tmx::TileLayer &layer,
                        const std::string &layerName) {
  const auto &layerTiles = layer.getTiles();
  const auto &tilesets = map.getTilesets();
  const auto &tileCount = map.getTileCount();

  unsigned int width = tileCount.x;
  unsigned int height = tileCount.y;

  // Get or create the vector for this layer
  std::vector<TileQuad> &currentLayerTiles = layerTiles_[layerName];

  for (unsigned int y = 0; y < height; ++y) {
    for (unsigned int x = 0; x < width; ++x) {
      std::size_t index = y * width + x;
      if (index >= layerTiles.size())
        continue;

      const auto &tile = layerTiles[index];
      if (tile.ID == 0)
        continue; // Empty tile

      // Find which tileset this tile belongs to
      const tmx::Tileset *usedTileset = nullptr;
      for (auto it = tilesets.rbegin(); it != tilesets.rend(); ++it) {
        if (tile.ID >= it->getFirstGID() && tile.ID < it->getLastGID()) {
          usedTileset = &(*it);
          break;
        }
      }

      if (!usedTileset || usedTileset->getImagePath().empty())
        continue;

      auto textureIt = textures_.find(usedTileset->getImagePath());
      if (textureIt == textures_.end())
        continue;

      const sf::Texture *texture = textureIt->second.get();
      auto tileSize = usedTileset->getTileSize();

      // Calculate tile position in world coordinates
      float worldX = x * tileSize_.x;
      float worldY = y * tileSize_.y;

      // Calculate tile position in tileset texture
      std::uint32_t localID = tile.ID - usedTileset->getFirstGID();
      unsigned int tilesPerRow = texture->getSize().x / tileSize.x;
      unsigned int tileX = localID % tilesPerRow;
      unsigned int tileY = localID / tilesPerRow;

      float texX = tileX * tileSize.x;
      float texY = tileY * tileSize.y;

      // Create quad for this tile
      TileQuad quad;
      quad.texture = texture;

      // Top-left
      quad.vertices[0] = sf::Vertex(sf::Vector2f(worldX, worldY),
                                    sf::Color::White, sf::Vector2f(texX, texY));

      // Top-right
      quad.vertices[1] =
          sf::Vertex(sf::Vector2f(worldX + tileSize.x, worldY),
                     sf::Color::White, sf::Vector2f(texX + tileSize.x, texY));

      // Bottom-right
      quad.vertices[2] = sf::Vertex(
          sf::Vector2f(worldX + tileSize.x, worldY + tileSize.y),
          sf::Color::White, sf::Vector2f(texX + tileSize.x, texY + tileSize.y));

      // Bottom-left
      quad.vertices[3] =
          sf::Vertex(sf::Vector2f(worldX, worldY + tileSize.y),
                     sf::Color::White, sf::Vector2f(texX, texY + tileSize.y));

      // Handle flip flags (horizontal, vertical, and diagonal)
      // Tiled encoding: diagonal = transpose texture coords, then apply H/V flips
      // Diagonal alone = 90° CW, Diagonal+H = 90° CCW, Diagonal+V = 180°, Diagonal+H+V = 270° CW
      bool hasHorizontal = tile.flipFlags & tmx::TileLayer::FlipFlag::Horizontal;
      bool hasVertical = tile.flipFlags & tmx::TileLayer::FlipFlag::Vertical;
      bool hasDiagonal = tile.flipFlags & tmx::TileLayer::FlipFlag::Diagonal;

      if (hasDiagonal) {
        // Store original texture coordinates
        sf::Vector2f origTex[4];
        for (int i = 0; i < 4; ++i) {
          origTex[i] = quad.vertices[i].texCoords;
        }
        
        // Calculate relative coordinates from tile origin (texX, texY)
        float dx0 = origTex[0].x - texX;
        float dy0 = origTex[0].y - texY;
        float dx1 = origTex[1].x - texX;
        float dy1 = origTex[1].y - texY;
        float dx2 = origTex[2].x - texX;
        float dy2 = origTex[2].y - texY;
        float dx3 = origTex[3].x - texX;
        float dy3 = origTex[3].y - texY;
        
        if (hasHorizontal && hasVertical) {
          // 270° CW: transpose (swap x/y), then flip both
          quad.vertices[0].texCoords = sf::Vector2f(texX + tileSize.y - dy0, texY + tileSize.x - dx0);
          quad.vertices[1].texCoords = sf::Vector2f(texX + tileSize.y - dy1, texY + tileSize.x - dx1);
          quad.vertices[2].texCoords = sf::Vector2f(texX + tileSize.y - dy2, texY + tileSize.x - dx2);
          quad.vertices[3].texCoords = sf::Vector2f(texX + tileSize.y - dy3, texY + tileSize.x - dx3);
        } else if (hasVertical) {
          // 180°: transpose, then flip vertically
          quad.vertices[0].texCoords = sf::Vector2f(texX + tileSize.y - dy0, texY + dx0);
          quad.vertices[1].texCoords = sf::Vector2f(texX + tileSize.y - dy1, texY + dx1);
          quad.vertices[2].texCoords = sf::Vector2f(texX + tileSize.y - dy2, texY + dx2);
          quad.vertices[3].texCoords = sf::Vector2f(texX + tileSize.y - dy3, texY + dx3);
        } else if (hasHorizontal) {
          // 90° CCW: transpose only
          quad.vertices[0].texCoords = sf::Vector2f(texX + dy0, texY + dx0);
          quad.vertices[1].texCoords = sf::Vector2f(texX + dy1, texY + dx1);
          quad.vertices[2].texCoords = sf::Vector2f(texX + dy2, texY + dx2);
          quad.vertices[3].texCoords = sf::Vector2f(texX + dy3, texY + dx3);
        } else {
          // 90° CW: transpose, then flip horizontally
          quad.vertices[0].texCoords = sf::Vector2f(texX + dy0, texY + tileSize.x - dx0);
          quad.vertices[1].texCoords = sf::Vector2f(texX + dy1, texY + tileSize.x - dx1);
          quad.vertices[2].texCoords = sf::Vector2f(texX + dy2, texY + tileSize.x - dx2);
          quad.vertices[3].texCoords = sf::Vector2f(texX + dy3, texY + tileSize.x - dx3);
        }
      } else {
        // No diagonal, just apply horizontal and vertical flips
        if (hasHorizontal) {
          std::swap(quad.vertices[0].texCoords.x, quad.vertices[1].texCoords.x);
          std::swap(quad.vertices[2].texCoords.x, quad.vertices[3].texCoords.x);
        }
        if (hasVertical) {
          std::swap(quad.vertices[0].texCoords.y, quad.vertices[3].texCoords.y);
          std::swap(quad.vertices[1].texCoords.y, quad.vertices[2].texCoords.y);
        }
      }

      // Add to both layer-specific and global tiles
      currentLayerTiles.push_back(quad);
      tiles_.push_back(quad);
    }
  }
}

sf::Color Track::getBackgroundColor() const {
  if (!mapLoaded_) {
    return sf::Color::Black; // Default to black if map not loaded
  }

  const auto &bgColor = map_.getBackgroundColour();
  if (bgColor.r == 0 && bgColor.g == 0 && bgColor.b == 0 && bgColor.a == 0) {
    // No background color set, default to black
    return sf::Color::Black;
  }

  // Convert tmxlite Colour to SFML Color
  return sf::Color(bgColor.r, bgColor.g, bgColor.b, bgColor.a);
}

void Track::draw(sf::RenderTarget &target, sf::RenderStates states) const {
  if (!mapLoaded_)
    return;

  // Group tiles by texture for efficient rendering
  std::map<const sf::Texture *, std::vector<sf::Vertex>> textureGroups;

  // Render layers in the order they appear in the TMX file (bottom to top)
  // This ensures correct z-ordering (e.g., checkpoints above track)
  for (const std::string &layerName : layerOrder_) {
    auto layerIt = layerTiles_.find(layerName);
    if (layerIt == layerTiles_.end()) {
      continue; // Layer not found (shouldn't happen)
    }

    std::string layerNameLower = layerName;
    std::transform(
        layerNameLower.begin(), layerNameLower.end(), layerNameLower.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    // Skip checkpoint activation layers (checkpoint_1, checkpoint_2, etc.)
    // unless activated
    bool isCheckpointLayer = layerNameLower.find("checkpoint_") == 0;
    if (isCheckpointLayer) {
      if (!activeCheckpointLayers_.contains(layerNameLower))
        continue;
    }

    // Add tiles from this layer to the render groups
    for (const auto &quad : layerIt->second) {
      if (quad.texture) {
        auto &vertices = textureGroups[quad.texture];
        // Add two triangles per quad
        vertices.push_back(quad.vertices[0]);
        vertices.push_back(quad.vertices[1]);
        vertices.push_back(quad.vertices[2]);
        vertices.push_back(quad.vertices[0]);
        vertices.push_back(quad.vertices[2]);
        vertices.push_back(quad.vertices[3]);
      }
    }
  }

  // Draw each texture group
  for (const auto &group : textureGroups) {
    states.texture = group.first;
    target.draw(group.second.data(), group.second.size(), sf::Triangles,
                states);
  }
}

std::unique_ptr<sf::Texture>
Track::extractTileTexture(std::uint32_t tileGID) const {
  if (tileGID == 0 || !mapLoaded_) {
    return nullptr;
  }

  const auto &tilesets = map_.getTilesets();
  const tmx::Tileset *usedTileset = nullptr;

  // Find which tileset this tile belongs to
  for (auto it = tilesets.rbegin(); it != tilesets.rend(); ++it) {
    if (tileGID >= it->getFirstGID() && tileGID < it->getLastGID()) {
      usedTileset = &(*it);
      break;
    }
  }

  if (!usedTileset || usedTileset->getImagePath().empty()) {
    return nullptr;
  }

  // Find the texture for this tileset
  auto textureIt = textures_.find(usedTileset->getImagePath());
  if (textureIt == textures_.end()) {
    return nullptr;
  }

  const sf::Texture *tilesetTexture = textureIt->second.get();
  auto tileSize = usedTileset->getTileSize();

  // Calculate tile position in tileset
  std::uint32_t localID = tileGID - usedTileset->getFirstGID();
  unsigned int tilesPerRow = tilesetTexture->getSize().x / tileSize.x;
  unsigned int tileX = localID % tilesPerRow;
  unsigned int tileY = localID / tilesPerRow;

  unsigned int texX = tileX * tileSize.x;
  unsigned int texY = tileY * tileSize.y;

  // Extract the tile region
  sf::Image tileImage = tilesetTexture->copyToImage();
  sf::Image extracted;
  extracted.create(tileSize.x, tileSize.y);

  for (unsigned int y = 0; y < tileSize.y && (texY + y) < tileImage.getSize().y;
       ++y) {
    for (unsigned int x = 0;
         x < tileSize.x && (texX + x) < tileImage.getSize().x; ++x) {
      extracted.setPixel(x, y, tileImage.getPixel(texX + x, texY + y));
    }
  }

  auto texture = std::make_unique<sf::Texture>();
  texture->loadFromImage(extracted);
  return texture;
}

std::vector<Track::BorderTile> Track::getBorderTiles() const {
  std::vector<BorderTile> borderTiles;

  if (!mapLoaded_)
    return borderTiles;

  const auto &layers = map_.getLayers();

  // Find the "border" layer (case-insensitive)
  const tmx::TileLayer *borderLayer = nullptr;
  for (const auto &layer : layers) {
    if (layer->getType() == tmx::Layer::Type::Tile) {
      std::string layerName = layer->getName();
      std::string layerNameLower = layerName;
      std::transform(layerNameLower.begin(), layerNameLower.end(),
                     layerNameLower.begin(),
                     [](unsigned char c) {
                       return static_cast<char>(std::tolower(c));
                     });

      if (layerNameLower == "border") {
        borderLayer = &layer->getLayerAs<tmx::TileLayer>();
        break;
      }
    }
  }

  if (!borderLayer)
    return borderTiles;

  const auto &layerTiles = borderLayer->getTiles();
  const auto &tileCount = map_.getTileCount();
  unsigned int width = tileCount.x;
  unsigned int height = tileCount.y;

  // Iterate through all tiles in the border layer
  for (unsigned int y = 0; y < height; ++y) {
    for (unsigned int x = 0; x < width; ++x) {
      std::size_t index = y * width + x;
      if (index >= layerTiles.size())
        continue;

      const auto &tile = layerTiles[index];
      if (tile.ID == 0)
        continue; // Skip empty tiles

      // Calculate tile position (center of tile)
      float tileCenterX = (x + 0.5f) * tileSize_.x;
      float tileCenterY = (y + 0.5f) * tileSize_.y;

      BorderTile borderTile;
      borderTile.position = {tileCenterX, tileCenterY};
      borderTile.size = {static_cast<float>(tileSize_.x),
                         static_cast<float>(tileSize_.y)};

      borderTiles.push_back(borderTile);
    }
  }

  return borderTiles;
}

bool Track::isPositionOnTrack(const sf::Vector2f &position) const {
  if (!mapLoaded_)
    return false;

  const auto &layers = map_.getLayers();

  // Find the "Track" layer (case-insensitive)
  const tmx::TileLayer *trackLayer = nullptr;
  for (const auto &layer : layers) {
    if (layer->getType() == tmx::Layer::Type::Tile) {
      std::string layerName = layer->getName();
      std::string layerNameLower = layerName;
      std::transform(layerNameLower.begin(), layerNameLower.end(),
                     layerNameLower.begin(),
                     [](unsigned char c) {
                       return static_cast<char>(std::tolower(c));
                     });

      if (layerNameLower == "track") {
        trackLayer = &layer->getLayerAs<tmx::TileLayer>();
        break;
      }
    }
  }

  if (!trackLayer)
    return false;

  // Convert pixel position to tile coordinates
  unsigned int tileX = static_cast<unsigned int>(position.x / tileSize_.x);
  unsigned int tileY = static_cast<unsigned int>(position.y / tileSize_.y);

  const auto &layerTiles = trackLayer->getTiles();
  const auto &tileCount = map_.getTileCount();
  unsigned int width = tileCount.x;
  unsigned int height = tileCount.y;

  // Check if tile coordinates are within bounds
  if (tileX >= width || tileY >= height)
    return false;

  // Check if there's a non-empty tile at this position
  std::size_t index = tileY * width + tileX;
  if (index >= layerTiles.size())
    return false;

  const auto &tile = layerTiles[index];
  return tile.ID != 0; // Non-zero ID means there's a tile
}
