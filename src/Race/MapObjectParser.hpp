#pragma once
#include <SFML/System/Vector2.hpp>
#include <string>
#include <tmxlite/Map.hpp>
#include <vector>

struct VehicleSpawnPoint {
  sf::Vector2f position;
  sf::Vector2f size;
  float rotation = 0.f;
  std::string name;
  std::string layerName;
  std::string textureId;
  int tileID = 0;
};

struct SoftObstacleData {
  int id = 0;
  sf::Vector2f position;
  sf::Vector2f size;

  float desiredStrength = 0.5f; // default oil values
  float accelerationStrength = 0.5f;
  float activationDuration = 2.0f;
  std::string type = "oil"; // "oil" or "booster"
};

struct CheckpointData {
  sf::Vector2f center;
  sf::Vector2f halfSize; // half width/height for Box2D
  int id = 0; // Checkpoint ID from TMX property (used for layer names)
};

class MapObjectParser {
public:
  MapObjectParser() = default;

  bool parse(const tmx::Map &map);

  const std::vector<VehicleSpawnPoint> &getVehicleSpawns() const {
    return vehicleSpawns_;
  }

  const std::vector<SoftObstacleData> &getSoftObstacles() const {
    return softObstacles_;
  }

  const std::vector<CheckpointData> &getCheckpoints() const {
    return checkpoints_;
  }

private:
  void parseVehicleObjects(const tmx::ObjectGroup &group);
  void parseSoftObstacleObjects(const tmx::ObjectGroup &group);
  void parseCheckpointObjects(const tmx::ObjectGroup &group);

private:
  std::vector<VehicleSpawnPoint> vehicleSpawns_;
  std::vector<SoftObstacleData> softObstacles_;
  std::vector<CheckpointData> checkpoints_;
};
