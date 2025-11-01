#include "PauseScene.hpp"
#include "Scenes/MenuScene.hpp"
#include "System/GameContext.hpp"
#include "System/SceneManager.hpp"
#include <iostream>

PauseScene::PauseScene(SceneManager &manager, GameContext &context)
    : manager_(manager), context_(context) {

  sf::RenderWindow &window = manager_.getWindow();
  // initialize restart game and exit buttons
  auto startGameBtn = std::make_unique<Button>("Resume"); // resume the race
  auto exitGameBtn =
      std::make_unique<Button>("End Game"); // go back to main menu

  startGameBtn->onPress = [&]() { manager_.popScene(); };

  exitGameBtn->onPress = [&]() { manager_.switchScene<MenuScene>(context_); };

  buttonsContainer_.addBtn(std::move(startGameBtn));
  buttonsContainer_.addBtn(std::move(exitGameBtn));

  // position buttons to the center of the window vertically
  buttonsContainer_.setScale();
  buttonsContainer_.setPosition(sf::Vector2f(
      window.getSize().x / 2.f - buttonsContainer_.getSize().x / 2.f,
      window.getSize().y / 2.f - buttonsContainer_.getSize().y / 2.f));

  buttonsContainer_.highlightBtn(0);
}

void PauseScene::handleEvent(const sf::Event &event) {
  if (event.type == sf::Event::KeyPressed) {
    if (event.key.code == sf::Keyboard::Up) {
      selectedIndex_ =
          (selectedIndex_ + buttonsContainer_.buttons_.size() - 1) %
          buttonsContainer_.buttons_.size();
      std::cout << selectedIndex_ << '\n';
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

void PauseScene::render() { buttonsContainer_.render(manager_.getWindow()); }
