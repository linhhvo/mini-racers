#include "HUD.hpp"
#include "Race/CheckpointManager.hpp"
#include "System/GameContext.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>

const std::string FONT_FILE{"../assets/fonts/MenuScenceFont.ttf"};
sf::Font HUDfont;

HUD::HUD(Timer &timer, GameContext &context,
         std::vector<CarProgress> &progresses)
    : timer_(timer), context_(context), progresses_(progresses) {
  if (!HUDfont.loadFromFile(FONT_FILE)) {
    std::cout << "unable to load font" << std::endl;
  }

  // initialize race timer text
  timerText_.setFont(HUDfont);
  timerText_.setCharacterSize(15);
  timerText_.setFillColor(sf::Color::Black);

  // initialize remaining lap text
  lapText_.setFont(HUDfont);
  lapText_.setCharacterSize(15);
  lapText_.setFillColor(sf::Color::Black);
}

void HUD::update() {
  // update timer text
  float remaining = timer_.getRemainingTime();
  int minutes = static_cast<int>(remaining) / 60;
  int seconds = static_cast<int>(remaining) % 60;

  std::ostringstream timer_ss;
  timer_ss << std::setw(2) << std::setfill('0') << minutes << ":"
           << std::setw(2) << std::setfill('0') << seconds;

  timerText_.setString(timer_ss.str());

  // update remaining lap text
  std::ostringstream lap_ss;
  for (unsigned int i = 0; i < context_.config->numOfPlayers; i++) {
    remainingNofLap_ = context_.config->numOfLaps - progresses_[i].currentLap;
    lap_ss << "P" << i + 1 << ": " << remainingNofLap_ << "/"
           << context_.config->numOfLaps << "\n";
  }
  lapText_.setString(lap_ss.str());
}

void HUD::setPosition(const sf::Vector2f &pos) { timerText_.setPosition(pos); }

void HUD::render(sf::RenderTarget &target) {
  // set text positions
  timerText_.setPosition(
      target.getSize().x - timerText_.getGlobalBounds().width - 50, 10);
  lapText_.setPosition(50, 10);

  target.draw(timerText_);
  target.draw(lapText_);
}

// sf::Text &HUD::getText() { return timerText_; }
