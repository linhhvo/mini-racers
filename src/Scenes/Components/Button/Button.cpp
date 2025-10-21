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
sf::Font font;

Button::Button(const std::string &label)
    : button_(sf::RectangleShape({200.f, 100.f})) {

  if (!font.loadFromFile(FONT_FILE)) {
    std::cout << "unable to load font" << std::endl;
  }

  label_.setString(label);
  label_.setFont(font);
  label_.setFillColor(sf::Color::Black);
}

void Button::setPosition(const sf::Vector2f &pos) {
  button_.setPosition(pos);

  // position label text to the center of button
  sf::FloatRect buttonBounds = button_.getGlobalBounds();
  sf::FloatRect labelBounds = label_.getGlobalBounds();

  label_.setPosition(buttonBounds.left + (buttonBounds.width / 2.f) -
                         (labelBounds.width / 2.f),
                     buttonBounds.top + (buttonBounds.height / 2.f) -
                         (labelBounds.height / 2.f));
  label_.move(0.f, -labelBounds.top);
}

sf::Vector2f Button::getSize() const {
  return sf::Vector2f(button_.getSize().x, button_.getSize().y);
}

void Button::trigger() {
  if (onPress)
    onPress();
}

void Button::render(sf::RenderTarget &renderTarget) {
  renderTarget.draw(button_);
  renderTarget.draw(label_);
}
