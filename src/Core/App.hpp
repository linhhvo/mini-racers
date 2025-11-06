#pragma once

#include "../System/GameContext.hpp"
#include <SFML/Graphics.hpp>

class App {
public:
  App(GameContext &context);
  void run(); // main loop
private:
  sf::RenderWindow window;
  GameContext &context_;
};
