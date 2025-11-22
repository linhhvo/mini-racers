#include "SoftObstacleManager.hpp"

void SoftObstacleManager::addObstacle(const SoftObstacle &obstacle) {
  obstacles_.push_back(obstacle);
}

void SoftObstacleManager::update(float dt) {
  for (auto &obstacle : obstacles_) {
    if (obstacle.activated) {
      obstacle.activationTimer -= dt;
      if (obstacle.activationTimer <= 0.f) {
        obstacle.activationTimer = 0.f;
        obstacle.activated = false;
      }
    }
  }
}

int SoftObstacleManager::checkActivation(const sf::Vector2f &pos) {
  int newlyActivated = -1;

  for (auto &obstacle : obstacles_) {
    if (obstacle.contains(pos)) {
      if (!obstacle.activated) {
        obstacle.activated = true;
        obstacle.activationTimer = obstacle.activationDuration;
        newlyActivated = obstacle.id;
      } else {
        obstacle.activationTimer = obstacle.activationDuration;
      }
    }
  }

  return newlyActivated;
}

std::vector<int> SoftObstacleManager::getActiveObstacleIDs() const {
  std::vector<int> ids;
  for (const auto &obs : obstacles_) {
    if (obs.activated) {
      ids.push_back(obs.id);
    }
  }
  return ids;
}
