#pragma once
#include "Scene.hpp"
#include "Scenes/SceneManager.hpp"
#include "Gameplay/iforce2d_car.hpp"
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

  b2WorldId world_;
  iforce2d_TopdownCar* car_;
};