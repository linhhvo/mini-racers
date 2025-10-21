#pragma once

#include "Button.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

class BtnContainer {
public:
  std::vector<Button> buttons_;

  BtnContainer() {};

  void addBtn(const Button &btn);
  void setPosition(const sf::Vector2f &pos);
  sf::Vector2f getSize() const;
  void render(sf::RenderTarget &renderTarget);

private:
  float spacing_ = 20.f;
};
