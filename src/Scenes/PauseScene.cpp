#include "PauseScene.hpp"
#include "Scenes/MenuScene.hpp"
#include "System/AudioManager.hpp"
#include "System/GameContext.hpp"
#include "System/SceneManager.hpp"
#include <SFML/System/Vector2.hpp>
#include <iostream>

PauseScene::PauseScene(SceneManager &manager, GameContext &context)
    : manager_(manager), context_(context) {

  sf::RenderWindow &window = manager_.getWindow();
  // initialize restart game and exit buttons
  auto startGameBtn = std::make_unique<Button>("Resume"); // resume the race
  auto exitGameBtn =
      std::make_unique<Button>("End Game"); // go back to main menu

  startGameBtn->onPress = [&]() { manager_.popScene(); };

  exitGameBtn->onPress = [&]() {
    AudioManager::Instance().SetMusicVolume(80.0f);
    AudioManager::Instance().PlayMusic("../assets/sounds/music/menu_music.wav");
    std::cout << "[Audio] Menu music started at 80%" << std::endl;
    manager_.switchScene<MenuScene>(context_);
  };

  buttonsContainer_.addBtn(std::move(startGameBtn));
  buttonsContainer_.addBtn(std::move(exitGameBtn));

  // position buttons to the center of the window vertically
  buttonsContainer_.setScale();
  buttonsContainer_.setPosition(sf::Vector2f(
      window.getSize().x / 2.f - buttonsContainer_.getSize().x / 2.f,
      window.getSize().y / 2.f - buttonsContainer_.getSize().y / 2.f));

  buttonsContainer_.highlightBtn(0);

  // create a blurred overlay background
  background_.setSize(sf::Vector2f(window.getSize().x, window.getSize().y));
  background_.setFillColor(sf::Color(0, 0, 0, 150));
}

void PauseScene::handleEvent(const sf::Event &event) {
  if (event.type == sf::Event::KeyPressed) {
    if (event.key.code == sf::Keyboard::Up) {
      selectedIndex_ =
          (selectedIndex_ + buttonsContainer_.buttons_.size() - 1) %
          buttonsContainer_.buttons_.size();
    }
    if (event.key.code == sf::Keyboard::Down) {
      selectedIndex_ = (selectedIndex_ + 1) % buttonsContainer_.buttons_.size();
    }

    buttonsContainer_.highlightBtn(selectedIndex_);

    if (event.key.code == sf::Keyboard::Enter) {
      (*buttonsContainer_.buttons_[selectedIndex_]).trigger();
    }
  }
}

void PauseScene::render() {
  manager_.getWindow().draw(background_);
  buttonsContainer_.render(manager_.getWindow());
}
