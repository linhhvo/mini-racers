#include "MapObjectParser.hpp"
#include <algorithm>
#include <cctype>

bool MapObjectParser::parse(const tmx::Map &map) {
  vehicleSpawns_.clear();
  softObstacles_.clear();
  checkpoints_.clear();

  const auto &layers = map.getLayers();

  for (const auto &layer : layers) {
    if (layer->getType() != tmx::Layer::Type::Object)
      continue;

    const auto &group = layer->getLayerAs<tmx::ObjectGroup>();
    std::string name = group.getName();
    std::string lower = name;

    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (lower.find("vehicle") != std::string::npos ||
        lower.find("car") != std::string::npos ||
        lower.find("spawn") != std::string::npos) {
      parseVehicleObjects(group);
    } else if (lower.find("softobstacle") != std::string::npos) {
      parseSoftObstacleObjects(group);
    } else if (lower.find("checkpoint") != std::string::npos) {
      parseCheckpointObjects(group);
    }
  }

  return true;
}

void MapObjectParser::parseVehicleObjects(const tmx::ObjectGroup &group) {
  for (const auto &object : group.getObjects()) {
    VehicleSpawnPoint vehicleSpawn;

    auto pos = object.getPosition();
    auto aabb = object.getAABB();

    vehicleSpawn.position = {pos.x, pos.y};
    vehicleSpawn.size = {aabb.width, aabb.height};
    vehicleSpawn.rotation = object.getRotation();
    vehicleSpawn.name = object.getName();
    vehicleSpawn.layerName = group.getName();
    vehicleSpawn.tileID = object.getTileID(); // for tileset-based texture

    // auto-assign tile texture if needed
    if (vehicleSpawn.tileID > 0 && vehicleSpawn.textureId.empty()) {
      vehicleSpawn.textureId = "tile_" + std::to_string(vehicleSpawn.tileID);
    }

    vehicleSpawns_.push_back(vehicleSpawn);
  }
}

void MapObjectParser::parseSoftObstacleObjects(const tmx::ObjectGroup &group) {
  for (const auto &object : group.getObjects()) {
    SoftObstacleData obstacle;

    auto pos = object.getPosition();
    auto aabb = object.getAABB();

    obstacle.position = {pos.x, pos.y};
    obstacle.size = {aabb.width, aabb.height};

    // default oil values already set in constructor

    // read TMX properties
    for (const auto &prop : object.getProperties()) {
      if (prop.getName() == "id" && prop.getType() == tmx::Property::Type::Int)
        obstacle.id = prop.getIntValue();

      if (prop.getName() == "type" &&
          prop.getType() == tmx::Property::Type::String) {
        obstacle.type = prop.getStringValue();
        std::transform(obstacle.type.begin(), obstacle.type.end(),
                       obstacle.type.begin(), ::tolower);
      }

      if (prop.getName() == "desiredStrength" &&
          prop.getType() == tmx::Property::Type::Float)
        obstacle.desiredStrength = prop.getFloatValue();

      if (prop.getName() == "accelerationStrength" &&
          prop.getType() == tmx::Property::Type::Float)
        obstacle.accelerationStrength = prop.getFloatValue();

      if (prop.getName() == "duration" &&
          prop.getType() == tmx::Property::Type::Float)
        obstacle.activationDuration = prop.getFloatValue();
    }

    // apply booster defaults if type is "booster"
    if (obstacle.type == "booster") {
      if (obstacle.desiredStrength == 0.5f)
        obstacle.desiredStrength = 20.f;

      if (obstacle.accelerationStrength == 0.5f)
        obstacle.accelerationStrength = 10.f;

      obstacle.activationDuration = 0.3f;
    }

    obstacle.id = (int)softObstacles_.size(); // auto ID if not set

    softObstacles_.push_back(obstacle);
  }
}

void MapObjectParser::parseCheckpointObjects(const tmx::ObjectGroup &group) {
  for (const auto &obj : group.getObjects()) {
    auto pos = obj.getPosition();
    auto aabb = obj.getAABB();

    CheckpointData cp;

    cp.halfSize = {aabb.width * 0.5f, aabb.height * 0.5f};
    cp.center = {pos.x + cp.halfSize.x, pos.y + cp.halfSize.y};
    cp.id = 0; // Default if no id property

    // Read the 'id' property from the TMX object
    for (const auto &prop : obj.getProperties()) {
      if (prop.getName() == "id" && prop.getType() == tmx::Property::Type::Int) {
        cp.id = prop.getIntValue();
        break;
      }
    }

    checkpoints_.push_back(cp);
  }
}
