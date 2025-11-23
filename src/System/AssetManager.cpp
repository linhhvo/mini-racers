#include "AssetManager.hpp"

#include <tmxlite/Map.hpp>
#include <tmxlite/Tileset.hpp>

#include <iostream>

AssetManager::AssetManager() { createFallbackTexture(); }

// Fallback Texture (Checkerboard Magenta/Black)
void AssetManager::createFallbackTexture() {
  missingTexture_ = std::make_unique<sf::Texture>();

  sf::Image img;
  img.create(32, 32);

  for (unsigned y = 0; y < 32; y++) {
    for (unsigned x = 0; x < 32; x++) {
      bool even = ((x / 8 + y / 8) % 2 == 0);
      img.setPixel(x, y, even ? sf::Color::Magenta : sf::Color::Black);
    }
  }

  missingTexture_->loadFromImage(img);
  missingTexture_->setSmooth(false);

  std::cout << "[AssetManager] Fallback texture created.\n";
}

// Load any normal texture (vehicles, UI, backgrounds)
sf::Texture *AssetManager::getTexture(const std::string &id) {
  // Check cache
  auto it = textureCache_.find(id);
  if (it != textureCache_.end())
    return it->second.get();

  // Load new texture
  auto tex = std::make_unique<sf::Texture>();
  if (!tex->loadFromFile(id)) {
    std::cerr << "WARNING: Failed to load texture: " << id
              << "  → Using fallback texture.\n";
    return missingTexture_.get();
  }

  tex->setSmooth(true);

  sf::Texture *result = tex.get();
  textureCache_[id] = std::move(tex);
  return result;
}

// Load a font
sf::Font *AssetManager::getFont(const std::string &id) {
  auto it = fontCache_.find(id);
  if (it != fontCache_.end())
    return it->second.get();

  auto font = std::make_unique<sf::Font>();
  if (!font->loadFromFile(id)) {
    std::cerr << "WARNING: Failed to load font: " << id
              << "  → Using fallback font.\n";
    return missingFont_.get();
  }

  sf::Font *result = font.get();
  fontCache_[id] = std::move(font);
  return result;
}

// Load tile textures by GID (used by TMX object tiles)
sf::Texture *AssetManager::getTileTexture(uint32_t gid, const tmx::Map &map) {
  if (gid == 0)
    return missingTexture_.get();

  // Check cache
  auto it = tileCache_.find(gid);
  if (it != tileCache_.end())
    return it->second.get();

  const auto &tilesets = map.getTilesets();
  const tmx::Tileset *usedTileset = nullptr;

  // Find tileset for this tile GID
  for (auto iter = tilesets.rbegin(); iter != tilesets.rend(); ++iter) {
    if (gid >= iter->getFirstGID() && gid < iter->getLastGID()) {
      usedTileset = &(*iter);
      break;
    }
  }

  if (!usedTileset) {
    std::cerr << "WARNING: No tileset found for GID: " << gid << "\n";
    return missingTexture_.get();
  }

  const std::string imagePath = usedTileset->getImagePath();
  if (imagePath.empty()) {
    std::cerr << "WARNING: Tileset image missing for GID: " << gid << "\n";
    return missingTexture_.get();
  }

  // Load tileset image
  auto tilesetIt = textureCache_.find(imagePath);
  const sf::Texture *tilesetTexture = nullptr;

  if (tilesetIt != textureCache_.end()) {
    tilesetTexture = tilesetIt->second.get();
  } else {
    auto tex = std::make_unique<sf::Texture>();

    if (!tex->loadFromFile(imagePath)) {
      // Try alternative path
      std::string fallback = "../assets/maps/" + imagePath;

      if (!tex->loadFromFile(fallback)) {
        std::cerr << "WARNING: Failed to load tileset image '" << imagePath
                  << "'\n";
        return missingTexture_.get();
      }
    }

    tex->setSmooth(false);
    tilesetTexture = tex.get();
    textureCache_[imagePath] = std::move(tex);
  }

  // Extract tile region
  auto tileSize = usedTileset->getTileSize();
  uint32_t localID = gid - usedTileset->getFirstGID();
  unsigned tilesPerRow = tilesetTexture->getSize().x / tileSize.x;

  unsigned tileX = localID % tilesPerRow;
  unsigned tileY = localID / tilesPerRow;

  unsigned texX = tileX * tileSize.x;
  unsigned texY = tileY * tileSize.y;

  sf::Image source = tilesetTexture->copyToImage();
  sf::Image tileImg;
  tileImg.create(tileSize.x, tileSize.y);

  for (unsigned y = 0; y < tileSize.y; ++y)
    for (unsigned x = 0; x < tileSize.x; ++x)
      tileImg.setPixel(x, y, source.getPixel(texX + x, texY + y));

  auto outTexture = std::make_unique<sf::Texture>();
  outTexture->loadFromImage(tileImg);

  sf::Texture *result = outTexture.get();
  tileCache_[gid] = std::move(outTexture);

  return result;
}
