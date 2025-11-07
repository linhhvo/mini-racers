#pragma once

#include "CheckpointManager.hpp"
#include "System/GameContext.hpp"
#include "Timer.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

class HUD {
public:
  HUD(Timer &timer, GameContext &context,
      std::vector<CarProgress> &progresses_);

  void update();
  void render(sf::RenderTarget &target);

  void setPosition(const sf::Vector2f &pos);

  sf::Text &getText();

private:
  Timer &timer_;
  GameContext &context_;
  std::vector<CarProgress> &progresses_;
  sf::Text timerText_;
  sf::Text lapText_;
  int remainingNofLap_ = context_.config->numOfLaps;
};
