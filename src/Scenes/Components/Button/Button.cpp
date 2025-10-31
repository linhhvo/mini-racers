#include "Button.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>
#include <iostream>
#include <string>

const std::string FONT_FILE{"../assets/fonts/MenuScenceFont.ttf"};
const std::string BTN_NORMAL_FILE{"../assets/buttons/normal.png"};
const std::string BTN_HIGHLIGHT_FILE{"../assets/buttons/highlight.png"};

sf::Font font;

Button::Button(const std::string &label) {

  if (!font.loadFromFile(FONT_FILE)) {
    std::cout << "unable to load font" << std::endl;
  }

  // if (!normalTexture_.loadFromFile(BTN_NORMAL_FILE)) {
  //   std::cout << "unable to load normal texture" << std::endl;
  // }

  // buttonSprite_.setTexture(normalTexture_);
  applyTexture(NORMAL);
  buttonSprite_.setScale(6, 2.5);

  label_.setString(label);
  label_.setFont(font);
  label_.setFillColor(sf::Color::Black);
}

void Button::setPosition(const sf::Vector2f &pos) {
  buttonSprite_.setPosition(pos);

  // position label text to the center of button
  sf::FloatRect buttonBounds = buttonSprite_.getGlobalBounds();
  sf::FloatRect labelBounds = label_.getGlobalBounds();

  label_.setPosition(buttonBounds.left + (buttonBounds.width / 2.f) -
                         (labelBounds.width / 2.f),
                     buttonBounds.top + (buttonBounds.height / 2.f) -
                         (labelBounds.height / 2.f));
  label_.move(0.f, -labelBounds.top);
}

sf::Vector2f Button::getSize() const {
  return sf::Vector2f(buttonSprite_.getGlobalBounds().width,
                      buttonSprite_.getGlobalBounds().height);
}

void Button::trigger() {
  if (onPress)
    onPress();
}

void Button::applyTexture(Button::ButtonState state) {
  if (state == HIGHLIGHT) {
    if (!highlightTexture_.loadFromFile(BTN_HIGHLIGHT_FILE)) {
      std::cout << "unable to load highlight texture" << std::endl;
    }
    buttonSprite_.setTexture(highlightTexture_);
  }

  if (state == NORMAL) {
    if (!normalTexture_.loadFromFile(BTN_NORMAL_FILE)) {
      std::cout << "unable to load normal texture" << std::endl;
    }
    buttonSprite_.setTexture(normalTexture_);
  }
}

void Button::render(sf::RenderTarget &renderTarget) {
  renderTarget.draw(buttonSprite_);
  renderTarget.draw(label_);
}
