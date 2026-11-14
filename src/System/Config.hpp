#pragma once

#include <SFML/Window/VideoMode.hpp>
#include <string>
class Config {
public:
  Config() {};
  sf::VideoMode desktop_ = sf::VideoMode::getDesktopMode();
  const unsigned windowHeight = desktop_.height * 0.6f;
  const unsigned windowWidth = desktop_.width * 0.6f;
  const int framerate = 60;
  const int numOfLaps = 2;
  const float raceDuration = 300;
  const int numOfPlayers = 2;

  const float PIXELS_PER_METER = 30.0f;
  const float DEGTORAD = 0.0174532925199432957f;
  const float RADTODEG = 57.295779513082320876f;

  std::string TMX_MAP{"../assets/maps/map_1.tmx"};
};
