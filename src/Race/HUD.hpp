#pragma once

#include "Timer.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>

class HUD {
public:
  HUD(Timer &timer);

  void update();
  void render(sf::RenderTarget &target);

  void setPosition(const sf::Vector2f &pos);

  sf::Text &getText();

private:
  Timer &timer_;
  sf::Text timerText_;
  sf::Text rankText_;
  sf::Text lapText_;
};
