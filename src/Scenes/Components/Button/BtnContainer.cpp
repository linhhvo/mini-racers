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
  float yPadding = 0.f;
  for (auto &btn : buttons_) {
    btn->setPosition(sf::Vector2f(pos.x, pos.y + yPadding));
    yPadding += btn->getSize().y + spacing_;
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
    width = std::max(width, btn->getSize().x);
    height += btn->getSize().y + spacing_;
  }

  if (!buttons_.empty()) {
    height -= spacing_;
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
