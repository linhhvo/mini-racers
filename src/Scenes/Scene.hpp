#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

class Scene {
public:
  virtual ~Scene() = default;
  virtual void handleEvent(const sf::Event &event) = 0;
  virtual void update(float dt) = 0;
  virtual void render() = 0;
};
