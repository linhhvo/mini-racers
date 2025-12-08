#include "SelectMapScene.hpp"
#include "Components/Button/BtnContainer.hpp"
#include "Components/Button/Button.hpp"
#include "Scenes/MenuScene.hpp"
#include "System/GameContext.hpp"
#include "System/SceneManager.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

const std::string IMAGE_FILE{"../assets/backgrounds/MenuScene.jpg"};
const std::string FONT_FILE{"../assets/fonts/MenuScenceFont.ttf"};
sf::Font customFont;

SelectMapScene::SelectMapScene(SceneManager &manager, GameContext &context)
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

  // list of buttons
  auto map1Btn = std::make_unique<Button>("Map 1");
  auto map2Btn = std::make_unique<Button>("Map 2");

  // apply the selected map to the game
  map1Btn->onPress = [&]() {
    context_.config->TMX_MAP = "../assets/maps/map1.tmx";
    confirmationText_.setString("Map 1 applied successfully");
  };

  map2Btn->onPress = [&]() {
    context_.config->TMX_MAP = "../assets/maps/map2.tmx";
    confirmationText_.setString("Map 2 applied successfully");
  };

  buttonsContainer_.addBtn(std::move(map1Btn));
  buttonsContainer_.addBtn(std::move(map2Btn));

  // position buttons to the center of the window vertically
  buttonsContainer_.setOrientation(BtnContainer::Orientation::HORIZONTAL);
  buttonsContainer_.setScale();
  buttonsContainer_.setPosition(sf::Vector2f(
      window.getSize().x / 2.f - buttonsContainer_.getSize().x / 2.f,
      window.getSize().y / 2.f - buttonsContainer_.getSize().y / 2.f));

  buttonsContainer_.highlightBtn(0);

  // display an instruction to return to MenuScene
  if (!customFont.loadFromFile(FONT_FILE)) {
    std::cout << "unable to load font" << std::endl;
  }
  infoText_.setFont(customFont);
  infoText_.setString("Press Escape to return to main menu");
  infoText_.setCharacterSize(15);
  infoText_.setFillColor(sf::Color::White);
  infoText_.setPosition(window.getSize().x / 2.f -
                            infoText_.getGlobalBounds().width / 2.f,
                        buttonsContainer_.getPosition().y - 50);

  // display confirmation text if a map is seletected
  confirmationText_.setFont(customFont);
  confirmationText_.setCharacterSize(15);
  confirmationText_.setFillColor(sf::Color::White);
}

void SelectMapScene::handleEvent(const sf::Event &event) {
  if (event.type == sf::Event::KeyPressed) {
    if (event.key.code == sf::Keyboard::Left) {
      selectedIndex_ =
          (selectedIndex_ + buttonsContainer_.buttons_.size() - 1) %
          buttonsContainer_.buttons_.size();
    }
    if (event.key.code == sf::Keyboard::Right) {
      selectedIndex_ = (selectedIndex_ + 1) % buttonsContainer_.buttons_.size();
    }

    buttonsContainer_.highlightBtn(selectedIndex_);

    if (event.key.code == sf::Keyboard::Enter) {
      (*buttonsContainer_.buttons_[selectedIndex_]).trigger();
      showConfirmation_ = true;
    }

    if (event.key.code == sf::Keyboard::Escape) {
      manager_.switchScene<MenuScene>(context_);
    }
  }
}

void SelectMapScene::render() {
  manager_.getWindow().draw(backgroundSprite_);

  buttonsContainer_.render(manager_.getWindow());
  manager_.getWindow().draw(infoText_);
  if (showConfirmation_) {
    confirmationText_.setPosition(
        manager_.getWindow().getSize().x / 2.f -
            confirmationText_.getGlobalBounds().width / 2.f,
        (buttonsContainer_.getPosition().y + buttonsContainer_.getSize().y) +
            30);
    manager_.getWindow().draw(confirmationText_);
  }
}
