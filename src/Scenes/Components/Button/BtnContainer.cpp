#include "BtnContainer.hpp"
#include "Scenes/Components/Button/Button.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

void BtnContainer::addBtn(std::unique_ptr<Button> btn) {
  buttons_.push_back(std::move(btn));
}

void BtnContainer::setPosition(const sf::Vector2f &pos) {
  position_ = pos;
  // arrange buttons vertically or horizontally
  sf::Vector2f startPos = pos;

  for (auto &btn : buttons_) {
    btn->setPosition(startPos);
    if (orientation_ == Orientation::VERTICAL) {
      startPos.y += btn->getSize().y + spacing_;
    } else {
      startPos.x += btn->getSize().x + spacing_;
    }
  }
}

void BtnContainer::setScale() {
  float width = 0.f;
  float height = 0.f;

  for (const auto &btn : buttons_) {
    width = std::max(width, btn->getLabel().getGlobalBounds().width);
    height = std::max(height, btn->getLabel().getGlobalBounds().height);
  }

  for (const auto &btn : buttons_) {
    float scaleX = width / btn->getSize().x;
    float scaleY = height / btn->getSize().y;

    btn->setScale(sf::Vector2f(scaleX + 1.5, scaleY + 1.5));
  }
}

sf::Vector2f BtnContainer::getSize() const {
  float width = 0.f;
  float height = 0.f;

  for (const auto &btn : buttons_) {
    if (orientation_ == Orientation::VERTICAL) {
      width = std::max(width, btn->getSize().x);
      height += btn->getSize().y + spacing_;
    } else {
      width += btn->getSize().x + spacing_;
      height = std::max(height, btn->getSize().y);
    }
  }

  if (!buttons_.empty()) {
    if (orientation_ == Orientation::VERTICAL) {
      height -= spacing_;
    } else {
      width -= spacing_;
    }
  }

  return sf::Vector2f(width, height);
}

void BtnContainer::highlightBtn(int selectedIdx) {
  for (unsigned int i = 0; i < buttons_.size(); i++) {
    if (i == selectedIdx) {
      buttons_[i]->applyTexture(Button::HIGHLIGHT);
    } else {
      buttons_[i]->applyTexture(Button::NORMAL);
    }
  }
}

void BtnContainer::render(sf::RenderTarget &renderTarget) {
  for (auto &btn : buttons_) {
    btn->render(renderTarget);
  }
}
