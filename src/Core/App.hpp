#pragma once
#include <SFML/Graphics.hpp>

class App {
public:
  App();
  void run(); // main loop
private:
  sf::RenderWindow window;
};
