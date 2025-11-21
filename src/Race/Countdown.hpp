#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>

class CountdownOverlay {
public:
  explicit CountdownOverlay(float seconds = 3.0f);

  // Restart countdown with specified seconds.
  void start(float seconds = 3.0f);

  // Advance timers; returns true exactly on the frame the countdown ends.
  bool update(float dt);

  // Skip world/input while counting down.
  bool isBlockingInput() const { return countingDown_; }

  void render(sf::RenderTarget &target);

private:
  void updateDisplay();

  float remaining_ = 0.0f;
  float goTimer_ = 0.0f;
  bool countingDown_ = true;
  sf::Font font_;
  sf::Text text_;
};
