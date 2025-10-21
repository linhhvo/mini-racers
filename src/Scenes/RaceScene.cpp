#include "RaceScene.hpp"
#include "Scenes/MenuScene.hpp"
#include "Scenes/SceneManager.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <iostream>

const std::string IMAGE_FILE{"../assets/backgrounds/RaceScene.jpg"};

RaceScene::RaceScene(SceneManager &manager) : manager_(manager) {
  sf::RenderWindow &window = manager_.getWindow();

  if (!backgroundTexture_.loadFromFile(IMAGE_FILE)) {
    std::cout << "unable to load background image file" << std::endl;
  }

  backgroundTexture_.setSmooth(true);
  backgroundSprite_.setTexture(backgroundTexture_);

  // center background image to the window's center and scale to fit window size
  backgroundSprite_.setOrigin(backgroundSprite_.getLocalBounds().width / 2.f,
                              backgroundSprite_.getLocalBounds().height / 2.f);
  backgroundSprite_.setScale(
      float(window.getSize().x) / backgroundSprite_.getLocalBounds().width,
      float(window.getSize().y) / backgroundSprite_.getLocalBounds().height);
  backgroundSprite_.setPosition(window.getSize().x / 2.f,
                                window.getSize().y / 2.f);
}

void RaceScene::handleEvent(const sf::Event &event) {
  if (event.type == sf::Event::KeyPressed) {
    if (event.key.code == sf::Keyboard::Escape) {
      manager_.switchScene<MenuScene>(manager_);
    }
  }
}

void RaceScene::render() { manager_.getWindow().draw(backgroundSprite_); }
