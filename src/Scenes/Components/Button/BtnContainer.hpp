#pragma once

#include "Button.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <memory>
#include <vector>

class BtnContainer {
public:
  enum class Orientation { VERTICAL, HORIZONTAL };
  std::vector<std::unique_ptr<Button>> buttons_;

  BtnContainer() {};

  void addBtn(std::unique_ptr<Button> btn);

  void setPosition(const sf::Vector2f &pos);
  void setScale();
  void setSpacing(float spacing) { spacing_ = spacing; }
  void setOrientation(Orientation orientation) { orientation_ = orientation; }
  sf::Vector2f getPosition() const { return position_; }

  sf::Vector2f getSize() const;

  void render(sf::RenderTarget &renderTarget);
  void highlightBtn(int selectedIdx);

private:
  float spacing_ = 20.f;
  Orientation orientation_ = Orientation::VERTICAL;
  sf::Vector2f position_{0.f, 0.f};
};
