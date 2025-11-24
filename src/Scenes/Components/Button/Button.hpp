#pragma once

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>
#include <functional>
#include <string>

class Button {
public:
  enum ButtonState { HIGHLIGHT, NORMAL };

  Button(const std::string &labelText);

  sf::Vector2f getSize() const;
  sf::Text &getLabel();

  void setPosition(const sf::Vector2f &pos);
  void setScale(const sf::Vector2f &factors);

  void render(sf::RenderTarget &renderTarget);
  void applyTexture(ButtonState state);

  void trigger();
  std::function<void()> onPress;

private:
  sf::Sprite buttonSprite_;
  sf::Texture normalTexture_;
  sf::Texture highlightTexture_;
  sf::Text label_;
};
