#include "CheckpointManager.hpp"
#include "System/GameContext.hpp"
#include <iostream>
#include <memory>
#include <ostream>
#include <sstream>

CheckpointManager::CheckpointManager(b2WorldId world, GameContext &context)
    : world_(world), context_(context) {
  carProgress_.resize(
      context_.config->numOfPlayers); // initialize progress for all cars
}

void CheckpointManager::loadFromData(
    const std::vector<CheckpointData> &checkpoints) {
  checkpoints_.clear();

  for (const auto &data : checkpoints) {
    b2Vec2 worldPos(data.center.x / context_.config->PIXELS_PER_METER,
                    data.center.y / context_.config->PIXELS_PER_METER);
    checkpoints_.push_back(std::make_unique<Checkpoint>(
        world_, worldPos, data.halfSize.x / context_.config->PIXELS_PER_METER,
        data.halfSize.y / context_.config->PIXELS_PER_METER));
  }
}

// Find checkpoint by Box2D shape
Checkpoint *CheckpointManager::FindCheckpointByShapeId(b2ShapeId shape) const {
  for (auto &cp : checkpoints_) {
    if (B2_ID_EQUALS(cp->GetShapeId(), shape)) {
      return cp.get();
    }
  }
  return nullptr;
}

int CheckpointManager::GetCheckpointIndex(const Checkpoint *cp) const {
  for (std::size_t i = 0; i < checkpoints_.size(); ++i) {
    if (checkpoints_[i].get() == cp) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

// Car hit checkpoint effect
void CheckpointManager::onCarHitCheckpoint(int carID, Checkpoint *cp) {
  if (carID < 0 || carID >= static_cast<int>(carProgress_.size()))
    return;

  int triggeredCheckpoint = GetCheckpointIndex(cp);
  if (triggeredCheckpoint < 0)
    return;

  CarProgress &progress = carProgress_[carID];

  cp->activate();

  progress.passedCheckpoints.insert(triggeredCheckpoint);

  if (triggeredCheckpoint == 0 &&
      progress.passedCheckpoints.size() == checkpoints_.size()) {
    progress.currentLap++;
    progress.passedCheckpoints.clear();

    if (progress.currentLap >= context_.config->numOfLaps) {
      raceFinished_ = true;
      winnerCarID_ = carID;
    }

    ResetCheckpoints();
  }
}

// Update all checkpoint animations
void CheckpointManager::update(float dt) {
  for (auto &cp : checkpoints_) {
    cp->update(dt);
  }
}

// Reset checkpoints
void CheckpointManager::ResetCheckpoints() {
  for (auto &cp : checkpoints_) {
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
  for (auto &cp : checkpoints_) {
    if (cp->isActivated()) {
      int i = GetCheckpointIndex(cp.get());
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
