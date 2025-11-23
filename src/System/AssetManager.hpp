#pragma once
#include "tmxlite/Map.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <unordered_map>

class AssetManager {
public:
  AssetManager();

  sf::Texture *getTexture(const std::string &id);
  sf::Texture *getTileTexture(uint32_t gid, const tmx::Map &map);

  sf::Font *getFont(const std::string &id);

  sf::Texture *getFallbackTexture() { return missingTexture_.get(); }

private:
  void createFallbackTexture();

  std::unordered_map<std::string, std::unique_ptr<sf::Texture>> textureCache_;
  std::unordered_map<uint32_t, std::unique_ptr<sf::Texture>> tileCache_;
  std::unordered_map<std::string, std::unique_ptr<sf::Font>> fontCache_;

  std::unique_ptr<sf::Texture> missingTexture_;
  std::unique_ptr<sf::Font> missingFont_;
};
