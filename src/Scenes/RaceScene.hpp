#pragma once
#include "Race/CheckpointManager.hpp"
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

struct CarStatus {
  bool inSoftObstacle = false;
  float timer = 0.0f;
};

public:
  RaceScene(SceneManager &manager, GameContext &context);
  void handleEvent(const sf::Event &event);
  void update(float dt);
  void render();

private:
  GameContext &context_;
  std::unique_ptr<CheckpointManager> checkpointManager;


  sf::Texture backgroundTexture_;
  sf::Sprite backgroundSprite_;
  SceneManager &manager_;

  sf::Texture carTexture_;
  std::vector<sf::Sprite> carSprites;

  sf::Texture obsTexture_;
  std::vector<sf::Sprite> obsSprites;

  sf::Texture chpTexture_;
  std::vector<sf::Sprite> checkpointSprites_;


  sf::Texture sofTexture_;
  std::vector<sf::Sprite> sofSprites;

  sf::Texture bosTexture_;
  std::vector<sf::Sprite> bosSprites;

  b2WorldId world_;
  iforce2d_TopdownCar *car_;
  std::vector<CarStatus> CarStatus_;
  InputManager input_;
  int nofCars = context_.config->numOfPlayers;
  int nofSoft_ = 2;
  int nofObstacles_ = 2;
  int nofBoosters_ = 3;
  int spacing = 10;

  Timer raceTimer_;
  std::unique_ptr<HUD> HUD_display_;

  std::vector<CarProgress> carProgress_;
  bool raceFinished_ = false;            // has any car finished the race?
  int winnerCarID_ = -1;   
};
