#pragma once
#include "Race/MapObjectParser.hpp"
#include "SoftObstacle.hpp"
#include <vector>

class SoftObstacleManager {
public:
  void loadFromData(const std::vector<SoftObstacleData> &data);

  void update(float dt);

  // Returns ID of newly activated obstacle (or -1)
  int checkActivation(const sf::Vector2f &position);

  std::vector<int> getActiveObstacleIDs() const;

  // For Track to provide obstacles parsed from TMX
  std::vector<SoftObstacle> &getAll() { return obstacles_; }

private:
  std::vector<SoftObstacle> obstacles_;
};
