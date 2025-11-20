#pragma once

#include "Gameplay/Checkpoints/Checkpoint.hpp"
#include "tmxlite/Map.hpp"
#include <box2d/box2d.h>
#include <set>
#include <vector>

struct CarProgress {
  std::set<int> passedCheckpoints;
  int currentLap = 0;
  bool finished = false;
};

class CheckpointManager {
public:
  CheckpointManager(b2WorldId world, int totalLaps, int numCars);
  ~CheckpointManager();

  // Load all checkpoints from TMX object layers whose name contains
  // "checkpoint"
  void LoadFromMap(const tmx::Map &map, float pixelsPerMeter);

  // Called by RaceScene when a sensor triggers
  void onCarHitCheckpoint(int carID, Checkpoint *checkpoint);

  Checkpoint *FindCheckpointByShapeId(b2ShapeId shape) const;

  int GetCheckpointIndex(const Checkpoint *cp) const;

  // Reset check activation (e.g., every lap)
  void ResetCheckpoints();

  void update(float dt);

  // detect race finish
  bool isRaceFinished() const { return raceFinished_; }
  int getWinnerCarID() const { return winnerCarID_; }

  std::vector<CarProgress> &getProgresses() { return carProgress_; }
  int getLapCount(int carID) const;

  std::set<std::string> getActivatedCheckpointID() const;

private:
  b2WorldId world_;
  std::vector<Checkpoint *> checkpoints_;
  std::vector<CarProgress> carProgress_;
  int totalLaps_;
  bool raceFinished_ = false;
  int winnerCarID_ = -1;
};
