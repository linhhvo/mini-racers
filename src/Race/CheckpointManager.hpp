#pragma once

#include "Gameplay/Checkpoint.hpp"
#include <box2d/box2d.h>
#include <vector>

struct CarProgress {
  int nextCheckpointIndex = 0; // next checkpoint in order
  int currentLap = 0;
  bool finished = false;
};

class CheckpointManager {
public:
  CheckpointManager(b2WorldId world, int totalLaps, int numCars);
  ~CheckpointManager();
  void AddCheckpoint(const b2Vec2 &position);
  std::vector<Checkpoint *> GetCheckpoints() const { return checkpoints_; }

private:
  b2WorldId world_;
  std::vector<Checkpoint *> checkpoints_;
  std::vector<CarProgress> carProgress_;
  int totalLaps_;
  bool raceFinished = false;
  int winnerCarID = -1;
};
