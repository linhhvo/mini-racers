#include "Countdown.hpp"
#include "System/AudioManager.hpp"
#include <cmath>
#include <iostream>

namespace {
const std::string FONT_FILE{"../assets/fonts/MenuScenceFont.ttf"};
}

CountdownOverlay::CountdownOverlay(float seconds) {
  if (!font_.loadFromFile(FONT_FILE)) {
    std::cerr << "unable to load countdown font: " << FONT_FILE << '\n';
  }
  text_.setFont(font_);
  text_.setCharacterSize(80);
  text_.setFillColor(sf::Color::White);
  text_.setOutlineColor(sf::Color::Black);
  text_.setOutlineThickness(3.0f);

  start(seconds);
}

void CountdownOverlay::start(float seconds) {
  remaining_ = seconds;
  goTimer_ = 0.0f;
  countingDown_ = true;
  lastDisplayedNumber_ = -1;
  updateDisplay();
}

bool CountdownOverlay::update(float dt) {
  bool finishedThisFrame = false;

  if (countingDown_) {
    remaining_ -= dt;
    if (remaining_ <= 0.0f) {
      countingDown_ = false;
      goTimer_ = 0.8f;
      text_.setString("GO!");
      // Play "GO!" sound
      AudioManager::Instance().PlaySound("../assets/sounds/audio/race_start_sound.wav");
      std::cout << "[Audio] Countdown GO!" << std::endl;
      finishedThisFrame = true;
    } else {
      updateDisplay();
      // Play beep sound when number changes
      int currentNumber = static_cast<int>(std::ceil(remaining_));
      if (currentNumber != lastDisplayedNumber_) {
        lastDisplayedNumber_ = currentNumber;
        AudioManager::Instance().PlaySound("../assets/sounds/audio/click_sound.wav");
        std::cout << "[Audio] Countdown beep: " << currentNumber << std::endl;
      }
    }
  } else if (goTimer_ > 0.0f) {
    goTimer_ -= dt;
  }

  return finishedThisFrame;
}

void CountdownOverlay::render(sf::RenderTarget &target) {
  if (!countingDown_ && goTimer_ <= 0.0f)
    return;

  sf::Vector2f center(target.getSize().x * 0.5f, target.getSize().y * 0.5f);
  sf::FloatRect bounds = text_.getLocalBounds();
  text_.setOrigin(bounds.width * 0.5f, bounds.height * 0.5f);
  text_.setPosition(center);
  target.draw(text_);
}

void CountdownOverlay::updateDisplay() {
  int display = static_cast<int>(std::ceil(remaining_));
  text_.setString(std::to_string(display));
}
