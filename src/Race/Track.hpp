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
    
private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    void createLayer(const tmx::Map& map, const tmx::TileLayer& layer);
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
    
    std::vector<TileQuad> tiles_;
    std::map<std::string, std::unique_ptr<sf::Texture>> textures_;
    std::vector<SpawnPoint> vehicleSpawns_;
};

