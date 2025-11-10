#include "CheckpointManager.hpp"

CheckpointManager::CheckpointManager(b2WorldId world, int totalLaps,
                                     int numCars)
    : world_(world), totalLaps_(totalLaps) {
  carProgress_.resize(numCars); // initialize progress for all cars
}

CheckpointManager::~CheckpointManager() {
  for (auto c : checkpoints_) {
    delete c;
  }
}

void CheckpointManager::AddCheckpoint(const b2Vec2 &position) {
  Checkpoint *cp = new Checkpoint(world_, position);
  checkpoints_.push_back(cp);
}
