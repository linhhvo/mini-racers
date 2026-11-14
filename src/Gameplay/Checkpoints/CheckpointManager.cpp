#include "CheckpointManager.hpp"
#include "System/GameContext.hpp"
#include <algorithm>
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
  checkpointIds_.clear();

  // Find the checkpoint with the highest ID (finish line) - it should be index 0
  int maxId = -1;
  int maxIdIndex = -1;
  for (size_t i = 0; i < checkpoints.size(); ++i) {
    if (checkpoints[i].id > maxId) {
      maxId = checkpoints[i].id;
      maxIdIndex = static_cast<int>(i);
    }
  }

  // First, add the finish line (highest ID) as index 0
  if (maxIdIndex >= 0) {
    const auto &data = checkpoints[maxIdIndex];
    b2Vec2 worldPos(data.center.x / context_.config->PIXELS_PER_METER,
                    data.center.y / context_.config->PIXELS_PER_METER);
    checkpoints_.push_back(std::make_unique<Checkpoint>(
        world_, worldPos, data.halfSize.x / context_.config->PIXELS_PER_METER,
        data.halfSize.y / context_.config->PIXELS_PER_METER));
    checkpointIds_.push_back(data.id);
  }

  // Then add all other checkpoints
  for (size_t i = 0; i < checkpoints.size(); ++i) {
    if (static_cast<int>(i) != maxIdIndex) {
      const auto &data = checkpoints[i];
      b2Vec2 worldPos(data.center.x / context_.config->PIXELS_PER_METER,
                      data.center.y / context_.config->PIXELS_PER_METER);
      checkpoints_.push_back(std::make_unique<Checkpoint>(
          world_, worldPos, data.halfSize.x / context_.config->PIXELS_PER_METER,
          data.halfSize.y / context_.config->PIXELS_PER_METER));
      checkpointIds_.push_back(data.id);
    }
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

  // Check if this is the finish line (checkpoint 0) and all other checkpoints have been passed
  if (triggeredCheckpoint == 0) {
    // For a lap to complete, we need to have passed all checkpoints EXCEPT checkpoint 0
    // (since checkpoint 0 is the finish line we're about to cross)
    int requiredCheckpoints = checkpoints_.size() - 1; // All checkpoints except 0
    
    // Count how many non-zero checkpoints have been passed
    int passedNonZeroCheckpoints = 0;
    for (int cpIndex : progress.passedCheckpoints) {
      if (cpIndex != 0) {
        passedNonZeroCheckpoints++;
      }
    }
    
    // If all non-zero checkpoints have been passed, complete the lap
    if (passedNonZeroCheckpoints == requiredCheckpoints) {
      progress.currentLap++;
      progress.passedCheckpoints.clear(); // Reset for next lap

      if (progress.currentLap >= context_.config->numOfLaps) {
        raceFinished_ = true;
        winnerCarID_ = carID;
        std::cout << "Car " << winnerCarID_ << " wins the race!\n";
      }

      ResetCheckpoints();
    }
  }
  
  // Add the checkpoint to passed set (after checking for lap completion)
  progress.passedCheckpoints.insert(triggeredCheckpoint);
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
  for (size_t i = 0; i < checkpoints_.size(); ++i) {
    if (checkpoints_[i]->isActivated()) {
      // Use the checkpoint's id property (from TMX) for the layer name
      int checkpointId = checkpointIds_[i];
      std::stringstream layerName;
      layerName << "checkpoint_" << checkpointId;
      // Ensure lowercase for consistent matching
      std::string name = layerName.str();
      std::transform(name.begin(), name.end(), name.begin(),
                     [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
      layers.insert(name);
    }
  }
  return layers;
}
