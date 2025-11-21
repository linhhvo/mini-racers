#include "CheckpointManager.hpp"
#include "System/GameContext.hpp"
#include <algorithm>
#include <iostream>
#include <ostream>
#include <sstream>

CheckpointManager::CheckpointManager(b2WorldId world, GameContext &context)
    : world_(world), context_(context) {
  carProgress_.resize(
      context_.config->numOfPlayers); // initialize progress for all cars
}

CheckpointManager::~CheckpointManager() {
  for (auto c : checkpoints_) {
    delete c;
  }
  checkpoints_.clear();
}

// load checkpoints from tmx map
void CheckpointManager::LoadFromMap(const tmx::Map &map) {
  const auto &layers = map.getLayers();

  std::cout << "CheckpointManager: scanning TMX for checkpoints...\n";

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

    std::cout << "  Found checkpoint layer: '" << name << "' with "
              << objectGroup.getObjects().size() << " objects\n";

    for (const auto &obj : objectGroup.getObjects()) {
      auto pos = obj.getPosition();
      auto aabb = obj.getAABB();

      float cpWidth = aabb.width * 0.5f;
      float cpHeight = aabb.height * 0.5f;

      float centerX = pos.x + cpWidth;
      float centerY = pos.y + cpHeight;

      b2Vec2 worldPos(centerX / context_.config->PIXELS_PER_METER,
                      centerY / context_.config->PIXELS_PER_METER);

      // Create checkpoint
      Checkpoint *cp = new Checkpoint(
          world_, worldPos, cpWidth / context_.config->PIXELS_PER_METER,
          cpHeight / context_.config->PIXELS_PER_METER);
      checkpoints_.push_back(cp);
    }
  }

  std::cout << "CheckpointManager: total checkpoints = " << checkpoints_.size()
            << "\n";
}

// Find checkpoint by Box2D shape
Checkpoint *CheckpointManager::FindCheckpointByShapeId(b2ShapeId shape) const {
  for (auto *cp : checkpoints_) {
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

// Car hit checkpoint (called by RaceScene sensor code)
void CheckpointManager::onCarHitCheckpoint(int carID, Checkpoint *cp) {
  if (carID < 0 || carID >= static_cast<int>(carProgress_.size()))
    return;

  int triggeredCheckpoint = GetCheckpointIndex(cp);
  if (triggeredCheckpoint < 0)
    return;

  CarProgress &progress = carProgress_[carID];

  cp->activate();

  progress.passedCheckpoints.insert(triggeredCheckpoint);
  std::cout << "passed checkpoint " << triggeredCheckpoint << std::endl;

  if (triggeredCheckpoint == 0 &&
      progress.passedCheckpoints.size() == checkpoints_.size()) {
    progress.currentLap++;
    progress.passedCheckpoints.clear();
    std::cout << "Car " << carID << " completed lap " << progress.currentLap
              << "\n";

    if (progress.currentLap >= totalLaps_) {
      raceFinished_ = true;
      winnerCarID_ = carID;
      std::cout << "Car " << winnerCarID_ << " wins the race!\n";
    }

    ResetCheckpoints();
  }
}

// Update all checkpoint animations
void CheckpointManager::update(float dt) {
  for (auto *cp : checkpoints_) {
    cp->update(dt);
  }
}

// Reset checkpoints for a new lap
void CheckpointManager::ResetCheckpoints() {
  for (auto *cp : checkpoints_) {
    cp->reset();
  }
}

int CheckpointManager::getLapCount(int carID) const {
  if (carID < 0 || carID >= static_cast<int>(carProgress_.size()))
    return 0;
  return carProgress_[carID].currentLap;
}

std::set<std::string> CheckpointManager::getActivatedCheckpointID() const {
  std::set<std::string> layers{};
  for (auto *cp : checkpoints_) {
    if (cp->isActivated()) {
      int i = GetCheckpointIndex(cp);
      std::stringstream layerName;
      if (i > 0) {
        layerName << "checkpoint_" << i;
      } else if (i == 0) {
        layerName << "checkpoint_" << 9;
      }
      layers.insert(layerName.str());
    }
  }
  return layers;
}
