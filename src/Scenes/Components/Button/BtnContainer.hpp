#pragma once

#include "Button.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <memory>
#include <vector>

class BtnContainer {
public:
  std::vector<std::unique_ptr<Button>> buttons_;

  BtnContainer() {};

  void addBtn(std::unique_ptr<Button> btn);
  void setPosition(const sf::Vector2f &pos);
  sf::Vector2f getSize() const;
  void render(sf::RenderTarget &renderTarget);

private:
  float spacing_ = 20.f;
};
