#pragma once

#include "Gameplay/Obstacles/SoftObstacleManager.hpp"
#include "Gameplay/Vehicle.hpp"
#include <SFML/System/Vector2.hpp>
#include <vector>

class SoftObstacleInteraction {
public:
  SoftObstacleInteraction(SoftObstacleManager *manager,
                          const std::vector<Vehicle *> &vehicles);

  // Called once per frame per car with updated world position
  void onCarPositionUpdated(int carIndex, const sf::Vector2f &position);

  // Updates timers & resets expired effects
  void update(float dt);

private:
  struct CarState {
    bool inObstacle = false;
    float timer = 0.f;
  };

  SoftObstacleManager *manager_;
  std::vector<Vehicle *> vehicles_;
  std::vector<CarState> carStates_;
};
