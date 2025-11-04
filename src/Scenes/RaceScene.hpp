#pragma once
#include "../System/SceneManager.hpp"
#include "Gameplay/iforce2d_car.hpp"
#include "Race/HUD.hpp"
#include "Race/Timer.hpp"
#include "Scene.hpp"
#include "System/GameContext.hpp"
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <memory>

class RaceScene : public Scene {
public:
  RaceScene(SceneManager &manager, GameContext &context);
  void handleEvent(const sf::Event &event);
  void update(float dt);
  void render();

private:
  GameContext &context_;

  sf::Texture backgroundTexture_;
  sf::Sprite backgroundSprite_;
  SceneManager &manager_;

  sf::Texture carTexture_;
  sf::Sprite carSprite_;
  std::vector<sf::Sprite> carSprites;

  sf::Texture obsTexture_;
  sf::Sprite obsSprite_;

  sf::Texture chpTexture_;
  sf::Sprite chpSprite_;

  sf::Texture sofTexture_;
  sf::Sprite sofSprite_;

  b2WorldId world_;
  iforce2d_TopdownCar *car_;
  InputManager input_;
  int nofCars = context_.config->numOfPlayers;
  int spacing = 100;

  Timer raceTimer_;
  std::unique_ptr<HUD> HUD_display_;
};
