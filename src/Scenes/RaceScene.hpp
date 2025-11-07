#pragma once
#include "../System/SceneManager.hpp"
#include "Gameplay/iforce2d_car.hpp"
#include "Race/CheckpointManager.hpp"
#include "Race/HUD.hpp"
#include "Race/Timer.hpp"
#include "Race/Track.hpp"
#include "Scene.hpp"
#include "System/GameContext.hpp"
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

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

  Track track_;
  bool useTmxMap_;

  SceneManager &manager_;

  std::vector<sf::Sprite> carSprites;
  std::unordered_map<std::string, std::unique_ptr<sf::Texture>>
      carTextureCache_;
  sf::Texture defaultCarTexture_;

  b2WorldId world_;
  iforce2d_TopdownCar *car_;
  std::vector<CarStatus> CarStatus_;
  InputManager input_;
  int nofSoft_ = 2;
  int nofObstacles_ = 2;
  int nofBoosters_ = 3;
  int spacing = 10;
  int nofCars = 0;
  // int nofCars = context_.config->numOfPlayers;

  Timer raceTimer_;
  std::unique_ptr<HUD> HUD_display_;

  std::vector<CarProgress> carProgress_;
  bool raceFinished_ = false; // has any car finished the race?
  int winnerCarID_ = -1;
  // Split screen views
  sf::View leftView_;  // For car 0 (WASD)
  sf::View rightView_; // For car 1 (Arrow keys)

  sf::Texture *getCarTexture(const std::string &textureId);
};
