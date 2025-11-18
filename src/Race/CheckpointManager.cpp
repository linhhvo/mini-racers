#include "CheckpointManager.hpp"
#include <algorithm>
#include <iostream>
#include <ostream>

CheckpointManager::CheckpointManager(b2WorldId world, int totalLaps,
                                     int numCars)
    : world_(world), totalLaps_(totalLaps) {
  // carProgress_.resize(numCars); // initialize progress for all cars
}

CheckpointManager::~CheckpointManager() {
  for (auto c : checkpoints_) {
    delete c;
  }
  checkpoints_.clear();
}

void CheckpointManager::AddCheckpoint(const b2Vec2 &position) {
  Checkpoint *cp = new Checkpoint(world_, position);
  checkpoints_.push_back(cp);
}

void CheckpointManager::LoadFromMap(const tmx::Map &map, float pixelsPerMeter) {
  const auto &layers = map.getLayers();

  std::cout << "CheckpointManager: scanning TMX for checkpoints...\n";

  for (const auto &layer : layers) {
    if (layer->getType() != tmx::Layer::Type::Object)
      continue;

    const auto &objectGroup = layer->getLayerAs<tmx::ObjectGroup>();
    std::string layerName = objectGroup.getName();
    std::string layerNameLower = layerName;
    std::transform(
        layerNameLower.begin(), layerNameLower.end(), layerNameLower.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (layerNameLower.find("checkpoint") == std::string::npos)
      continue;

    std::cout << "  Found checkpoint layer: '" << layerName << "' with "
              << objectGroup.getObjects().size() << " objects\n";

    for (const auto &object : objectGroup.getObjects()) {
      auto pos = object.getPosition();
      auto aabb = object.getAABB();

      float centerX = pos.x + aabb.width * 0.5f;
      float centerY = pos.y + aabb.height * 0.5f;

      b2Vec2 worldPos(centerX / pixelsPerMeter, centerY / pixelsPerMeter);

      std::cout << "    Adding checkpoint at pixels (" << centerX << ", "
                << centerY << "), world (" << worldPos.x << ", " << worldPos.y
                << ")\n";

      AddCheckpoint(worldPos);
    }
  }

  std::cout << "CheckpointManager: total checkpoints = " << checkpoints_.size()
            << '\n';
}

Checkpoint *CheckpointManager::FindCheckpointByShapeId(b2ShapeId shape) const {
  for (Checkpoint *cp : checkpoints_) {
    if (B2_ID_EQUALS(cp->GetShapeId(), shape)) {
      return cp;
    }
  }
  return nullptr;
}

int CheckpointManager::GetCheckpointIndex(const Checkpoint *cp) const {
  for (std::size_t i = 0; i < checkpoints_.size(); ++i) {
    if (checkpoints_[i] == cp) {
      return static_cast<int>(i);
    }
  }
  return -1;
}
