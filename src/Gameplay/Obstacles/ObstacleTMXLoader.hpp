#include "SoftObstacle.hpp"
#include "SoftObstacleManager.hpp"
#include "tmxlite/Map.hpp"
#include <algorithm>

class ObstacleTMXLoader {
public:
  static void loadSoftObstacles(SoftObstacleManager &manager,
                                const tmx::Map &map) {
    const auto &layers = map.getLayers();

    for (const auto &layer : layers) {
      if (layer->getType() != tmx::Layer::Type::Object)
        continue;

      const auto &objGroup = layer->getLayerAs<tmx::ObjectGroup>();
      std::string name = objGroup.getName();

      std::string lower = name;
      std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
      if (lower.find("softobstacle") == std::string::npos)
        continue;

      for (const auto &object : objGroup.getObjects()) {

        SoftObstacle obstacle;

        auto pos = object.getPosition();
        auto aabb = object.getAABB();
        obstacle.position = {pos.x, pos.y};
        obstacle.size = {aabb.width, aabb.height};

        obstacle.id = (int)manager.getAll().size();

        std::string type = "oil";

        for (const auto &prop : object.getProperties()) {

          if (prop.getName() == "id" &&
              prop.getType() == tmx::Property::Type::Int)
            obstacle.id = prop.getIntValue();

          if (prop.getName() == "type" &&
              prop.getType() == tmx::Property::Type::String) {
            type = prop.getStringValue();
            std::transform(type.begin(), type.end(), type.begin(), ::tolower);
          }

          if (prop.getName() == "desiredStrength" &&
              prop.getType() == tmx::Property::Type::Float)
            obstacle.desiredStrength = prop.getFloatValue();

          if (prop.getName() == "accelerationStrength" &&
              prop.getType() == tmx::Property::Type::Float)
            obstacle.accelerationStrength = prop.getFloatValue();
        }

        // default properties of each soft obstacle type
        if (type == "booster") {
          if (obstacle.desiredStrength == 0.5f)
            obstacle.desiredStrength = 20.f;
          if (obstacle.accelerationStrength == 0.5f)
            obstacle.accelerationStrength = 10.f;
          obstacle.activationDuration = 0.3f;
        } else { // oil
          if (obstacle.desiredStrength == 0.5f)
            obstacle.desiredStrength = 0.5f;
          if (obstacle.accelerationStrength == 0.5f)
            obstacle.accelerationStrength = 0.5f;
          obstacle.activationDuration = 2.0f;
        }

        manager.addObstacle(obstacle);
      }
    }
  }
};
