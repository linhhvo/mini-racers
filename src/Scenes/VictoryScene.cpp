#include "VictoryScene.hpp"
#include "Scenes/MenuScene.hpp"
#include "System/GameContext.hpp"
#include "System/SceneManager.hpp"
#include <SFML/System/Vector2.hpp>
#include <iostream>
#include <sstream>

const std::string BANNER_IMG{"../assets/others/banner.png"};
const std::string FONT_FILE{"../assets/fonts/MenuScenceFont.ttf"};
sf::Font VictoryFont;

VictoryScene::VictoryScene(SceneManager &manager, GameContext &context,
                           int winner)
    : manager_(manager), context_(context), winner_(winner) {
  sf::RenderWindow &window = manager_.getWindow();

  // create a blurred overlay background
  background_.setSize(sf::Vector2f(window.getSize().x, window.getSize().y));
  background_.setFillColor(sf::Color(0, 0, 0, 150));

  // apply banner image
  if (!bannerTexture_.loadFromFile(BANNER_IMG)) {
    std::cout << "unable to load background image file" << std::endl;
  }

  bannerSprite_.setTexture(bannerTexture_);
  bannerSprite_.setScale(8, 15);
  bannerSprite_.setPosition(
      window.getSize().x / 2.f - bannerSprite_.getGlobalBounds().width / 2.f,
      window.getSize().y / 2.f - bannerSprite_.getGlobalBounds().height / 2.f);

  // display an instruction to return to MenuScene
  if (!VictoryFont.loadFromFile(FONT_FILE)) {
    std::cout << "unable to load font" << std::endl;
  }

  infoText_.setFont(VictoryFont);
  infoText_.setString("Press Enter to return to main menu");
  infoText_.setCharacterSize(15);
  infoText_.setFillColor(sf::Color::White);
  infoText_.setPosition(
      window.getSize().x / 2.f - infoText_.getGlobalBounds().width / 2.f,
      (bannerSprite_.getPosition().y + bannerSprite_.getGlobalBounds().height) +
          20);

  // display race result
  std::stringstream result_ss;
  if (winner > -1) {
    result_ss << "Player " << winner + 1 << " won the race";
  }

  raceResult_.setFont(VictoryFont);
  raceResult_.setString(result_ss.str());
  raceResult_.setCharacterSize(13);
  raceResult_.setFillColor(sf::Color::Black);
  raceResult_.setPosition(bannerSprite_.getPosition().x + 100,
                          bannerSprite_.getPosition().y + 100);
}

void VictoryScene::handleEvent(const sf::Event &event) {
  if (event.type == sf::Event::KeyPressed) {
    if (event.key.code == sf::Keyboard::Enter) {
      manager_.switchScene<MenuScene>(context_);
    }
  }
}

void VictoryScene::render() {
  manager_.getWindow().draw(background_);
  if (winner_ > -1) {
    manager_.getWindow().draw(bannerSprite_);
  }
  manager_.getWindow().draw(infoText_);
  manager_.getWindow().draw(raceResult_);
}
