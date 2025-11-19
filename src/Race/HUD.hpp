#pragma once

#include "../Gameplay/Checkpoints/CheckpointManager.hpp"
#include "System/GameContext.hpp"
#include "Timer.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>

class HUD {
public:
  HUD(Timer &timer, GameContext &context);

  void setCheckpointManager(CheckpointManager *mgr) {
    checkpointManager_ = mgr;
  }

  void update();
  void render(sf::RenderTarget &target);

  void setPosition(const sf::Vector2f &pos);

  sf::Text &getText();

private:
  Timer &timer_;
  GameContext &context_;
  CheckpointManager *checkpointManager_ = nullptr;
  sf::Text timerText_;
  sf::Text lapText_;
  int remainingNofLap_ = context_.config->numOfLaps;
};
