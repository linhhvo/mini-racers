#pragma once

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>
#include <functional>
#include <string>

class Button {
public:
  Button(const std::string &labelText);
  sf::Vector2f getSize() const;
  void setPosition(const sf::Vector2f &pos);
  void render(sf::RenderTarget &renderTarget);
  void trigger();
  std::function<void()> onPress;

private:
  sf::RectangleShape button_;
  sf::Text label_;
};
