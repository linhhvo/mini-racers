#include "SoftObstacleInteraction.hpp"
#include "SoftObstacleManager.hpp"
#include <iostream>

SoftObstacleInteraction::SoftObstacleInteraction(
    SoftObstacleManager *manager, const std::vector<Vehicle *> &vehicles)
    : manager_(manager), vehicles_(vehicles) {
  carStates_.resize(vehicles.size());
}

void SoftObstacleInteraction::onCarPositionUpdated(
    int carIndex, const sf::Vector2f &position) {
  if (!manager_)
    return;

  int activatedId = manager_->checkActivation(position);

  // Newly activated obstacle
  if (activatedId >= 0) {
    auto &obstacles = manager_->getAll();

    for (auto &obstacle : obstacles) {
      if (obstacle.id != activatedId)
        continue;

      // Apply gameplay effect only if car wasn't already inside
      if (!carStates_[carIndex].inObstacle) {
        vehicles_[carIndex]->HitSoftObstacle(obstacle);

        std::cout << "Car " << carIndex << " hit obstacle " << activatedId
                  << "\n";
      }

      // Refresh / set timer
      carStates_[carIndex].inObstacle = true;
      carStates_[carIndex].timer = obstacle.activationDuration;
      break;
    }
  }

  // If car is inside any active obstacle, refresh timer
  if (carStates_[carIndex].inObstacle) {
    const auto &obstacles = manager_->getAll();

    for (const auto &obstacle : obstacles) {
      if (!obstacle.activated)
        continue;

      bool inside = (position.x >= obstacle.position.x &&
                     position.x <= obstacle.position.x + obstacle.size.x &&
                     position.y >= obstacle.position.y &&
                     position.y <= obstacle.position.y + obstacle.size.y);

      if (inside) {
        carStates_[carIndex].timer = obstacle.activationDuration;
        return;
      }
    }
  }
}

void SoftObstacleInteraction::update(float dt) {
  for (size_t i = 0; i < carStates_.size(); i++) {
    auto &state = carStates_[i];

    if (!state.inObstacle)
      continue;

    state.timer -= dt;

    if (state.timer <= 0.f) {
      state.inObstacle = false;
      state.timer = 0.f;

      vehicles_[i]->Reset();

      std::cout << "Car " << i << " returns to normal!\n";
    }
  }
}
