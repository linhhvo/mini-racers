#pragma once
#include <SFML/System.hpp>

class Timer {
public:
  Timer(float seconds)
      : duration_(seconds), remaining_(seconds), running_(false) {};

  void start() {
    running_ = true;
    clock_.restart();
  };

  void reset() {
    remaining_ = duration_;
    running_ = false;
  };

  void update(float dt) {
    if (!running_)
      return;

    // float elapsed = clock_.getElapsedTime().asSeconds();
    remaining_ -= dt;

    if (remaining_ <= 0.f) {
      remaining_ = 0.f;
      running_ = false;
    }
  }

  float getRemainingTime() const { return remaining_; };

  bool isFinished() const { return remaining_ <= 0.f; };

private:
  float duration_;
  sf::Clock clock_;
  float remaining_;
  bool running_;
};
