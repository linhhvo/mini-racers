#include "SoftObstacleManager.hpp"
#include "Race/MapObjectParser.hpp"

void SoftObstacleManager::loadFromData(
    const std::vector<SoftObstacleData> &data) {
  obstacles_.clear();
  obstacles_.reserve(data.size());

  for (const auto &d : data) {
    SoftObstacle obstacle;
    obstacle.id = d.id;
    obstacle.position = d.position;
    obstacle.size = d.size;
    obstacle.desiredStrength = d.desiredStrength;
    obstacle.accelerationStrength = d.accelerationStrength;
    obstacle.activationDuration = d.activationDuration;
    obstacle.type = d.type;
    obstacles_.push_back(obstacle);
  }
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
  for (const auto &obstacle : obstacles_) {
    if (obstacle.activated) {
      ids.push_back(obstacle.id);
    }
  }
  return ids;
}
