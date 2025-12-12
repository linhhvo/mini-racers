#pragma once

#include "Gameplay/Checkpoints/Checkpoint.hpp"
#include "Race/MapObjectParser.hpp"
#include "System/GameContext.hpp"
#include <box2d/box2d.h>
#include <memory>
#include <set>
#include <vector>

struct CarProgress {
  std::set<int> passedCheckpoints;
  int currentLap = 0;
  bool finished = false;
};

class CheckpointManager {
public:
  CheckpointManager(b2WorldId world, GameContext &context);
  ~CheckpointManager() = default;

  // load checkpoint objects from parsed map data
  void loadFromData(const std::vector<CheckpointData> &cps);

  void onCarHitCheckpoint(int carID, Checkpoint *checkpoint);

  Checkpoint *FindCheckpointByShapeId(b2ShapeId shape) const;

  int GetCheckpointIndex(const Checkpoint *cp) const;

  // Reset check activation
  void ResetCheckpoints();

  void update(float dt);

  // detect race finish
  bool isRaceFinished() const { return raceFinished_; }
  int getWinnerCarID() const { return winnerCarID_; }

  std::vector<CarProgress> &getProgresses() { return carProgress_; }
  int getLapCount(int carID) const;

  std::set<std::string> getActivatedCheckpointID() const;

private:
  GameContext &context_;
  b2WorldId world_;
  std::vector<std::unique_ptr<Checkpoint>> checkpoints_;
  std::vector<int> checkpointIds_; // Store checkpoint IDs (from TMX) corresponding to checkpoints_
  std::vector<CarProgress> carProgress_;
  bool raceFinished_ = false;
  int winnerCarID_ = -1;
};
