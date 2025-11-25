#include "MenuScene.hpp"
#include "Components/Button/BtnContainer.hpp"
#include "Components/Button/Button.hpp"
#include "RaceScene.hpp"
#include "SelectMapScene.hpp"
#include "System/GameContext.hpp"
#include "System/SceneManager.hpp"
#include "System/AudioManager.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <memory>
#include <string>

const std::string IMAGE_FILE{"../assets/backgrounds/MenuScene.jpg"};

MenuScene::MenuScene(SceneManager &manager, GameContext &context)
    : manager_(manager), context_(context) {
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

  // initialize start game and exit buttons
  auto selectOptionsBtn = std::make_unique<Button>("Options");
  auto startGameBtn = std::make_unique<Button>("Start");
  auto exitGameBtn = std::make_unique<Button>("Exit");

  // triggered events when a button is selected
  selectOptionsBtn->onPress = [&]() {
    AudioManager::Instance().PlaySound("../assets/sounds/sound/click_sound.wav");
    std::cout << "[Audio] Click sound played" << std::endl;
    manager_.switchScene<SelectMapScene>(context_);
  };
  startGameBtn->onPress = [&]() { 
    AudioManager::Instance().PlaySound("../assets/sounds/sound/click_sound.wav");
    std::cout << "[Audio] Click sound played" << std::endl;
    AudioManager::Instance().StopMusic();
    std::cout << "[Audio] Menu music stopped" << std::endl;
    manager_.switchScene<RaceScene>(context); 
  };
  exitGameBtn->onPress = [&]() { 
    AudioManager::Instance().PlaySound("../assets/sounds/sound/click_sound.wav");
    std::cout << "[Audio] Click sound played" << std::endl;
    window.close(); 
  };

  // add buttons to container
  buttonsContainer_.addBtn(std::move(selectOptionsBtn));
  buttonsContainer_.addBtn(std::move(startGameBtn));
  buttonsContainer_.addBtn(std::move(exitGameBtn));

  // position buttons to the center of the window vertically
  buttonsContainer_.setScale();
  buttonsContainer_.setPosition(sf::Vector2f(
      window.getSize().x / 2.f - buttonsContainer_.getSize().x / 2.f,
      window.getSize().y / 2.f - buttonsContainer_.getSize().y / 2.f));

  buttonsContainer_.highlightBtn(0);
  
  // Load and play menu music (continue if already playing, otherwise start)
  AudioManager::Instance().SetMusicVolume(80.0f);  // Set menu music to 80%
  // Only restart music if it's not already played 
  static bool musicStarted = false;
  if (!musicStarted) {
    AudioManager::Instance().PlayMusic("../assets/sounds/music/menu_music.wav");
    musicStarted = true;
    std::cout << "[Audio] Menu music started at 80%" << std::endl;
  } else {
    std::cout << "[Audio] Menu music continued at 80%" << std::endl;
  }
}

void MenuScene::handleEvent(const sf::Event &event) {
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

void MenuScene::render() {
  manager_.getWindow().draw(backgroundSprite_);

  buttonsContainer_.render(manager_.getWindow());
}
