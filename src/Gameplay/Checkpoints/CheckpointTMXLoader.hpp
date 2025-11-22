#pragma once

#include "Gameplay/Checkpoints/CheckpointManager.hpp"
#include "tmxlite/Map.hpp"
#include <algorithm>

class CheckpointTMXLoader {
public:
  static void loadCheckpoints(CheckpointManager &manager, const tmx::Map &map) {
    const auto &layers = map.getLayers();

    for (const auto &layer : layers) {
      if (layer->getType() != tmx::Layer::Type::Object)
        continue;

      const auto &objectGroup = layer->getLayerAs<tmx::ObjectGroup>();
      std::string name = objectGroup.getName();
      std::string lower = name;
      std::transform(
          lower.begin(), lower.end(), lower.begin(),
          [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

      if (lower.find("checkpoint") == std::string::npos)
        continue;

      for (const auto &obj : objectGroup.getObjects()) {
        auto pos = obj.getPosition();
        auto aabb = obj.getAABB();

        float cpWidth = aabb.width * 0.5f;
        float cpHeight = aabb.height * 0.5f;

        float centerX = pos.x + cpWidth;
        float centerY = pos.y + cpHeight;

        manager.createCheckpoint(cpWidth, cpHeight, centerX, centerY);
      }
    }
  };
};
