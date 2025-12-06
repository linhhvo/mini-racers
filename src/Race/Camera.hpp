#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

class Camera {
public:
  Camera() {};

  void configure(unsigned int numPlayers, const sf::Vector2u &windowSize);
  void setCarTarget(int cameraIndex, const sf::Vector2f &position);
  void updateViews();

  const sf::View &getView(int index) const;
  int getNumViews() const { return views_.size(); }

  void setZoom(float zoom);
  void resize(const sf::Vector2u &newSize);

private:
  void configureViewports(const sf::Vector2u &windowSize);

  std::vector<sf::View> views_;
  std::vector<sf::Vector2f> targets_;
  float zoomFactor_;
};
