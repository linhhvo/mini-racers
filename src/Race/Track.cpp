#include "Track.hpp"
#include <tmxlite/Tileset.hpp>
#include <tmxlite/ObjectGroup.hpp>
#include <tmxlite/Object.hpp>
#include <tmxlite/Property.hpp>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cctype>
#include <string>

Track::Track() : mapLoaded_(false), mapSize_(0, 0), tileSize_(0, 0) {
}

bool Track::loadFromFile(const std::string& filepath) {
    if (!map_.load(filepath)) {
        std::cout << "Failed to load TMX map: " << filepath << std::endl;
        return false;
    }
    
    if (map_.getOrientation() != tmx::Orientation::Orthogonal) {
        std::cout << "Map is not orthogonal - only orthogonal maps are supported" << std::endl;
        return false;
    }
    
    // Check if tilesets loaded properly
    const auto& tilesets = map_.getTilesets();
    if (tilesets.empty()) {
        std::cout << "Error: No tilesets found in map. Make sure tilesets are embedded in the TMX file." << std::endl;
        return false;
    }
    
    bool hasValidTileset = false;
    for (const auto& tileset : tilesets) {
        if (!tileset.getImagePath().empty()) {
            hasValidTileset = true;
            break;
        }
    }
    
    if (!hasValidTileset) {
        std::cout << "Error: No valid tilesets with images found." << std::endl;
        std::cout << "Hint: In Tiled, right-click your tileset -> 'Embed Tileset' to embed it in the TMX file." << std::endl;
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
    const auto& layers = map_.getLayers();
    for (const auto& layer : layers) {
        if (layer->getType() == tmx::Layer::Type::Tile) {
            std::string layerName = layer->getName();
            createLayer(map_, layer->getLayerAs<tmx::TileLayer>(), layerName);
            // Store layer order to preserve TMX rendering order (bottom to top)
            layerOrder_.push_back(layerName);
        }
    }

    parseObjectLayers(map_);
    if (!vehicleSpawns_.empty()) {
        std::cout << "Loaded " << vehicleSpawns_.size()
                  << " vehicle spawn point(s) from TMX." << std::endl;
    }
    
    std::cout << "Successfully loaded TMX map: " << filepath << std::endl;
    std::cout << "Map size: " << mapSize_.x << "x" << mapSize_.y << std::endl;
    std::cout << "Tile size: " << tileSize_.x << "x" << tileSize_.y << std::endl;
    
    return true;
}

void Track::loadTextures(const tmx::Map& map) {
    textures_.clear();
    
    const auto& tilesets = map.getTilesets();
    for (const auto& tileset : tilesets) {
        const std::string& imagePath = tileset.getImagePath();
        
        if (imagePath.empty()) {
            std::cout << "Warning: Tileset '" << tileset.getName() << "' has no image path" << std::endl;
            std::cout << "  This usually means the tileset is external (.tsx file) and wasn't found." << std::endl;
            std::cout << "  Solution: In Tiled, right-click the tileset -> 'Embed Tileset' to embed it in the TMX file." << std::endl;
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
                std::cout << "Warning: Failed to load tileset image: " << imagePath << std::endl;
                std::cout << "  Tried: " << imagePath << " and " << fullPath << std::endl;
                // Create a fallback magenta texture
                img.create(32, 32, sf::Color::Magenta);
            }
        }
        
        if (img.getSize().x > 0 && img.getSize().y > 0) {
            // Handle transparency
            if (tileset.hasTransparency()) {
                auto transparency = tileset.getTransparencyColour();
                img.createMaskFromColor({transparency.r, transparency.g, transparency.b, transparency.a});
            }
            texture->loadFromImage(img);
            textures_[imagePath] = std::move(texture);
            std::cout << "Loaded tileset image: " << imagePath << std::endl;
        }
    }
}

void Track::createLayer(const tmx::Map& map, const tmx::TileLayer& layer, const std::string& layerName) {
    const auto& layerTiles = layer.getTiles();
    const auto& tilesets = map.getTilesets();
    const auto& tileCount = map.getTileCount();
    
    unsigned int width = tileCount.x;
    unsigned int height = tileCount.y;
    
    // Get or create the vector for this layer
    std::vector<TileQuad>& currentLayerTiles = layerTiles_[layerName];
    
    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            std::size_t index = y * width + x;
            if (index >= layerTiles.size()) continue;
            
            const auto& tile = layerTiles[index];
            if (tile.ID == 0) continue; // Empty tile
            
            // Find which tileset this tile belongs to
            const tmx::Tileset* usedTileset = nullptr;
            for (auto it = tilesets.rbegin(); it != tilesets.rend(); ++it) {
                if (tile.ID >= it->getFirstGID() && tile.ID < it->getLastGID()) {
                    usedTileset = &(*it);
                    break;
                }
            }
            
            if (!usedTileset || usedTileset->getImagePath().empty()) continue;
            
            auto textureIt = textures_.find(usedTileset->getImagePath());
            if (textureIt == textures_.end()) continue;
            
            const sf::Texture* texture = textureIt->second.get();
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
            quad.vertices[0] = sf::Vertex(
                sf::Vector2f(worldX, worldY),
                sf::Color::White,
                sf::Vector2f(texX, texY)
            );
            
            // Top-right
            quad.vertices[1] = sf::Vertex(
                sf::Vector2f(worldX + tileSize.x, worldY),
                sf::Color::White,
                sf::Vector2f(texX + tileSize.x, texY)
            );
            
            // Bottom-right
            quad.vertices[2] = sf::Vertex(
                sf::Vector2f(worldX + tileSize.x, worldY + tileSize.y),
                sf::Color::White,
                sf::Vector2f(texX + tileSize.x, texY + tileSize.y)
            );
            
            // Bottom-left
            quad.vertices[3] = sf::Vertex(
                sf::Vector2f(worldX, worldY + tileSize.y),
                sf::Color::White,
                sf::Vector2f(texX, texY + tileSize.y)
            );
            
            // Handle flip flags (simplified - just horizontal and vertical)
            if (tile.flipFlags & tmx::TileLayer::FlipFlag::Horizontal) {
                std::swap(quad.vertices[0].texCoords.x, quad.vertices[1].texCoords.x);
                std::swap(quad.vertices[2].texCoords.x, quad.vertices[3].texCoords.x);
            }
            if (tile.flipFlags & tmx::TileLayer::FlipFlag::Vertical) {
                std::swap(quad.vertices[0].texCoords.y, quad.vertices[3].texCoords.y);
                std::swap(quad.vertices[1].texCoords.y, quad.vertices[2].texCoords.y);
            }
            
            // Add to both layer-specific and global tiles
            currentLayerTiles.push_back(quad);
            tiles_.push_back(quad);
        }
    }
}

void Track::update(float dt) {
    // Update checkpoint activation timers
    updateCheckpoints(dt);
}

sf::Color Track::getBackgroundColor() const {
    if (!mapLoaded_) {
        return sf::Color::Black; // Default to black if map not loaded
    }
    
    const auto& bgColor = map_.getBackgroundColour();
    if (bgColor.r == 0 && bgColor.g == 0 && bgColor.b == 0 && bgColor.a == 0) {
        // No background color set, default to black
        return sf::Color::Black;
    }
    
    // Convert tmxlite Colour to SFML Color
    return sf::Color(bgColor.r, bgColor.g, bgColor.b, bgColor.a);
}

void Track::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!mapLoaded_) return;
    
    // Get activated checkpoint IDs
    std::vector<int> activatedCheckpoints = getActivatedCheckpoints();
    
    // Group tiles by texture for efficient rendering
    std::map<const sf::Texture*, std::vector<sf::Vertex>> textureGroups;
    
    // Render layers in the order they appear in the TMX file (bottom to top)
    // This ensures correct z-ordering (e.g., checkpoints above track)
    for (const std::string& layerName : layerOrder_) {
        auto layerIt = layerTiles_.find(layerName);
        if (layerIt == layerTiles_.end()) {
            continue; // Layer not found (shouldn't happen)
        }
        
        std::string layerNameLower = layerName;
        std::transform(layerNameLower.begin(), layerNameLower.end(), layerNameLower.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        
        // Skip checkpoint activation layers (checkpoint_1, checkpoint_2, etc.) unless activated
        bool isCheckpointLayer = layerNameLower.find("checkpoint_") == 0;
        if (isCheckpointLayer) {
            // Extract checkpoint ID from layer name (e.g., "checkpoint_1" -> 1)
            try {
                std::string idStr = layerNameLower.substr(11); // "checkpoint_" is 11 chars
                int checkpointId = std::stoi(idStr);
                // Only render if this checkpoint is activated
                bool isActivated = std::find(activatedCheckpoints.begin(), activatedCheckpoints.end(), checkpointId) != activatedCheckpoints.end();
                if (!isActivated) {
                    continue; // Skip this layer
                }
            } catch (...) {
                // If we can't parse the ID, skip this layer
                continue;
            }
        }
        
        // Add tiles from this layer to the render groups
        for (const auto& quad : layerIt->second) {
            if (quad.texture) {
                auto& vertices = textureGroups[quad.texture];
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
    for (const auto& group : textureGroups) {
        states.texture = group.first;
        target.draw(group.second.data(), group.second.size(), sf::Triangles, states);
    }
}

std::unique_ptr<sf::Texture> Track::extractTileTexture(std::uint32_t tileGID) const {
    if (tileGID == 0 || !mapLoaded_) {
        return nullptr;
    }
    
    const auto& tilesets = map_.getTilesets();
    const tmx::Tileset* usedTileset = nullptr;
    
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
    
    const sf::Texture* tilesetTexture = textureIt->second.get();
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
    
    for (unsigned int y = 0; y < tileSize.y && (texY + y) < tileImage.getSize().y; ++y) {
        for (unsigned int x = 0; x < tileSize.x && (texX + x) < tileImage.getSize().x; ++x) {
            extracted.setPixel(x, y, tileImage.getPixel(texX + x, texY + y));
        }
    }
    
    auto texture = std::make_unique<sf::Texture>();
    texture->loadFromImage(extracted);
    return texture;
}

void Track::parseObjectLayers(const tmx::Map& map) {
    vehicleSpawns_.clear();
    checkpointZones_.clear();

    const auto &layers = map.getLayers();
    std::cout << "Parsing object layers... Found " << layers.size() << " total layers." << std::endl;
    
    for (const auto &layer : layers) {
        if (layer->getType() != tmx::Layer::Type::Object) {
            continue;
        }

        const auto &objectGroup = layer->getLayerAs<tmx::ObjectGroup>();
        std::string layerName = objectGroup.getName();
        std::cout << "Found object layer: '" << layerName << "' with " << objectGroup.getObjects().size() << " objects" << std::endl;
        
        std::string layerNameLower = layerName;
        std::transform(layerNameLower.begin(), layerNameLower.end(), layerNameLower.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        bool layerLooksLikeVehicles = layerNameLower.find("vehicle") != std::string::npos ||
                                      layerNameLower.find("car") != std::string::npos ||
                                      layerNameLower.find("spawn") != std::string::npos;
        
        bool layerIsCheckpoints = layerNameLower.find("checkpoint") != std::string::npos;

        for (const auto &object : objectGroup.getObjects()) {
            // Parse checkpoint zones
            if (layerIsCheckpoints) {
                CheckpointZone checkpoint;
                auto position = object.getPosition();
                auto aabb = object.getAABB();
                checkpoint.position = {position.x, position.y};
                checkpoint.size = {aabb.width, aabb.height};
                checkpoint.activated = false;
                checkpoint.id = -1; // Default invalid ID
                
                // Parse properties
                for (const auto &prop : object.getProperties()) {
                    std::cout << "    Checkpoint Property: " << prop.getName() << " = ";
                    if (prop.getType() == tmx::Property::Type::Int) {
                        std::cout << prop.getIntValue() << std::endl;
                        if (prop.getName() == "id") {
                            checkpoint.id = prop.getIntValue();
                        }
                    } else if (prop.getType() == tmx::Property::Type::Boolean) {
                        std::cout << (prop.getBoolValue() ? "true" : "false") << std::endl;
                        if (prop.getName() == "activated") {
                            checkpoint.activated = prop.getBoolValue();
                        }
                    } else if (prop.getType() == tmx::Property::Type::String) {
                        std::cout << prop.getStringValue() << std::endl;
                        if (prop.getName() == "type" && prop.getStringValue() == "checkpoint") {
                            // Valid checkpoint type
                        }
                    }
                }
                
                if (checkpoint.id >= 0) {
                    std::cout << "    -> INCLUDED as checkpoint zone (ID: " << checkpoint.id << ")" << std::endl;
                    checkpointZones_.push_back(std::move(checkpoint));
                } else {
                    std::cout << "    -> SKIPPED (no valid checkpoint ID)" << std::endl;
                }
                continue; // Skip vehicle parsing for checkpoint objects
            }
            
            // Parse vehicle spawns (existing logic)
            SpawnPoint spawn;
            auto position = object.getPosition();
            auto aabb = object.getAABB();
            spawn.position = {position.x, position.y};
            spawn.size = {aabb.width, aabb.height};
            spawn.rotation = object.getRotation();
            spawn.name = object.getName();
            spawn.layerName = layerName;
            spawn.tileID = object.getTileID(); // Get tile ID if object uses a tile

            std::cout << "  Object: name='" << spawn.name 
                      << "', tileID=" << spawn.tileID
                      << ", position=(" << spawn.position.x << "," << spawn.position.y << ")"
                      << ", size=(" << spawn.size.x << "," << spawn.size.y << ")" << std::endl;

            for (const auto &prop : object.getProperties()) {
                std::cout << "    Property: " << prop.getName() << " = " << prop.getStringValue() << std::endl;
                if (prop.getName() == "texture" && prop.getType() == tmx::Property::Type::String) {
                    spawn.textureId = prop.getStringValue();
                }
            }

            // If object has a tile ID but no texture property, use tile ID as texture identifier
            if (spawn.tileID > 0 && spawn.textureId.empty()) {
                spawn.textureId = "tile_" + std::to_string(spawn.tileID);
                std::cout << "    Auto-assigned textureId: " << spawn.textureId << std::endl;
            }

            // Include all objects from vehicle-like layers, or objects with texture property or tile ID
            bool includeObject = layerLooksLikeVehicles || !spawn.textureId.empty() || spawn.tileID > 0;
            
            if (includeObject) {
                std::cout << "    -> INCLUDED as vehicle spawn" << std::endl;
                vehicleSpawns_.push_back(std::move(spawn));
            } else {
                std::cout << "    -> SKIPPED (not a vehicle)" << std::endl;
            }
        }
    }
    
    std::cout << "Total vehicle spawns found: " << vehicleSpawns_.size() << std::endl;
    std::cout << "Total checkpoint zones found: " << checkpointZones_.size() << std::endl;
}

int Track::checkAndActivateCheckpoint(const sf::Vector2f& position) {
    int newlyActivated = -1;
    
    for (auto& checkpoint : checkpointZones_) {
        // Check if position is inside the checkpoint zone
        bool isInside = (position.x >= checkpoint.position.x &&
                         position.x <= checkpoint.position.x + checkpoint.size.x &&
                         position.y >= checkpoint.position.y &&
                         position.y <= checkpoint.position.y + checkpoint.size.y);
        
        if (isInside) {
            // Car is in the zone - activate or keep activated, reset timer
            if (!checkpoint.activated) {
                // Newly activated
                checkpoint.activated = true;
                checkpoint.activationTimer = CheckpointZone::ACTIVATION_DURATION;
                newlyActivated = checkpoint.id;
                std::cout << "Checkpoint " << checkpoint.id << " activated!" << std::endl;
            } else {
                // Already activated, reset timer to full duration
                checkpoint.activationTimer = CheckpointZone::ACTIVATION_DURATION;
            }
        }
        // If car is not in zone, timer will count down in updateCheckpoints()
    }
    
    return newlyActivated; // Return -1 if no new checkpoint activated
}

void Track::updateCheckpoints(float dt) {
    for (auto& checkpoint : checkpointZones_) {
        if (checkpoint.activated) {
            // Count down the timer
            checkpoint.activationTimer -= dt;
            
            // If timer expires, deactivate the checkpoint
            if (checkpoint.activationTimer <= 0.0f) {
                checkpoint.activated = false;
                checkpoint.activationTimer = 0.0f;
                std::cout << "Checkpoint " << checkpoint.id << " deactivated (timer expired)" << std::endl;
            }
        }
    }
}

std::vector<int> Track::getActivatedCheckpoints() const {
    std::vector<int> activated;
    for (const auto& checkpoint : checkpointZones_) {
        if (checkpoint.activated) {
            activated.push_back(checkpoint.id);
        }
    }
    return activated;
}

