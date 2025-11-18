#pragma once

#include "Gameplay/Checkpoint.hpp"
#include "tmxlite/Map.hpp"
#include <box2d/box2d.h>
#include <set>
#include <vector>

struct CarProgress {
  // int nextCheckpointIndex = 0; // next checkpoint in order
  std::set<int> passedCheckpoints;
  int currentLap = 0;
  bool finished = false;
};

class CheckpointManager {
public:
  CheckpointManager(b2WorldId world, int totalLaps, int numCars);
  ~CheckpointManager();

  void AddCheckpoint(const b2Vec2 &position);
  const std::vector<Checkpoint *> &GetCheckpoints() const {
    return checkpoints_;
  }

  // Load all checkpoints from TMX object layers whose name contains
  // "checkpoint"
  void LoadFromMap(const tmx::Map &map, float pixelsPerMeter);

  Checkpoint *FindCheckpointByShapeId(b2ShapeId shape) const;

  int GetCheckpointIndex(const Checkpoint *cp) const;

private:
  b2WorldId world_;
  std::vector<Checkpoint *> checkpoints_;
  // std::vector<CarProgress> carProgress_;
  int totalLaps_;
  bool raceFinished = false;
  int winnerCarID = -1;
};
