#pragma once

#include <SFML/Window/VideoMode.hpp>
class Config {
public:
  Config() {};
  sf::VideoMode desktop_ = sf::VideoMode::getDesktopMode();
  unsigned windowHeight = desktop_.height * 0.6f;
  unsigned windowWidth = desktop_.width * 0.6f;
  int framerate = 60;
  int numOfLaps = 1;
  float raceDuration = 300;
  int numOfPlayers = 2;
};
