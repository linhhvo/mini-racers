#include "HUD.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <iomanip>
#include <iostream>

const std::string FONT_FILE{"../assets/fonts/MenuScenceFont.ttf"};
sf::Font HUDfont;

HUD::HUD(Timer &timer) : timer_(timer) {
  if (!HUDfont.loadFromFile(FONT_FILE)) {
    std::cout << "unable to load font" << std::endl;
  }

  timerText_.setFont(HUDfont);
  timerText_.setCharacterSize(10);
  timerText_.setFillColor(sf::Color::White);
}

void HUD::update() {
  float remaining = timer_.getRemainingTime();
  int minutes = static_cast<int>(remaining) / 60;
  int seconds = static_cast<int>(remaining) % 60;

  std::ostringstream oss;
  oss << std::setw(2) << std::setfill('0') << minutes << ":" << std::setw(2)
      << std::setfill('0') << seconds;

  timerText_.setString(oss.str());
}

void HUD::setPosition(const sf::Vector2f &pos) { timerText_.setPosition(pos); }

void HUD::render(sf::RenderTarget &target) {
  timerText_.setPosition(target.getSize().x - 80, 10);

  target.draw(timerText_);
}

sf::Text &HUD::getText() { return timerText_; }
