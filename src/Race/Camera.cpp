#include "Camera.hpp"

void Camera::configure(unsigned int numPlayers,
                       const sf::Vector2u &windowSize) {
  views_.clear();
  targets_.clear();
  views_.resize(numPlayers);
  targets_.resize(numPlayers);
  configureViewports(windowSize);
}

void Camera::setZoom(float zoom) {
  zoomFactor_ = zoom;
  for (auto &v : views_)
    v.setSize(v.getSize() * zoomFactor_);
}

void Camera::setCarTarget(int index, const sf::Vector2f &pos) {
  if (index >= 0 && index < (int)targets_.size())
    targets_[index] = pos;
}

void Camera::updateViews() {
  for (int i = 0; i < (int)views_.size(); ++i)
    views_[i].setCenter(targets_[i]);
}

void Camera::configureViewports(const sf::Vector2u &windowSize) {
  if (views_.size() == 1) {
    // Full screen view
    views_[0].setViewport({0.f, 0.f, 1.f, 1.f});
  } else if (views_.size() == 2) {
    // Two-player vertical split
    views_[0].setViewport({0.f, 0.f, 0.5f, 1.f});
    views_[1].setViewport({0.5f, 0.f, 0.5f, 1.f});
  }

  // Set size independently of viewport
  for (auto &v : views_) {
    v.setSize(windowSize.x * 0.5f, windowSize.y);
  }
}

const sf::View &Camera::getView(int index) const { return views_[index]; }

void Camera::resize(const sf::Vector2u &newSize) {
  configureViewports(newSize);
}
