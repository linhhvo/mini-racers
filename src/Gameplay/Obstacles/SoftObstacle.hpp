#pragma once
#include <SFML/System/Vector2.hpp>
#include <string>

struct SoftObstacle {
  int id = 0;
  sf::Vector2f position;
  sf::Vector2f size;

  float desiredStrength = 0.5f;
  float accelerationStrength = 0.5f;

  float activationDuration = 2.0f;
  float activationTimer = 0.0f;
  bool activated = false;

  std::string type; // "oil", "booster", etc.

  bool contains(const sf::Vector2f &pos) const {
    return pos.x >= position.x && pos.x <= position.x + size.x &&
           pos.y >= position.y && pos.y <= position.y + size.y;
  }
};
