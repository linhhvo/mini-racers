#pragma once
#include "Gameplay/iforce2d_car.hpp"
#include "Scene.hpp"
#include "Scenes/SceneManager.hpp"
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

class RaceScene : public Scene {
public:
  RaceScene(SceneManager &manager);
  void handleEvent(const sf::Event &event);
  void update(float dt);
  void render();

private:
  sf::Texture backgroundTexture_;
  sf::Sprite backgroundSprite_;
  SceneManager &manager_;

  sf::Texture carTexture_;
  sf::Sprite carSprite_;
  std::vector<sf::Sprite> carSprites;

  b2WorldId world_;
  iforce2d_TopdownCar *car_;
  InputManager input_;
  int nofCars = 2;
  int spacing = 100;
};
