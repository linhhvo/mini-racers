#pragma once
#include "../System/SceneManager.hpp"
#include "Gameplay/Checkpoints/CheckpointManager.hpp"
#include "Gameplay/iforce2d_car.hpp"
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

public:
  struct CarStatus {
    bool inSoftObstacle = false;
    float timer = 0.0f;
  };
  RaceScene(SceneManager &manager, GameContext &context);
  void handleEvent(const sf::Event &event);
  void update(float dt);
  void render();

private:
  // counters
  int nofSoft_ = 2;
  int nofBoosters_ = 3;
  int nofCars_ = 0;
  int nofObstacles_ = 0;

  // system
  InputManager input_;
  GameContext &context_;

  // checkpoints
  std::unique_ptr<CheckpointManager> checkpointManager_;
  std::vector<CarProgress> carProgress_;
  bool raceFinished_ = false; // has any car finished the race?
  int winnerCarID_ = -1;

  // map
  Track track_;
  bool useTmxMap_;

  // scene
  SceneManager &manager_;

  // vehicles
  std::vector<sf::Sprite> carSprites;
  std::unordered_map<std::string, std::unique_ptr<sf::Texture>>
      carTextureCache_;
  std::vector<CarStatus> CarStatus_;
  sf::Texture defaultCarTexture_;

  // obstacles
  std::vector<sf::Sprite> obstacleSprites;
  std::unordered_map<std::string, std::unique_ptr<sf::Texture>>
      obstacleTextureCache_;
  std::vector<CarStatus> obstacleStatus_;
  sf::Texture defaultObstacleTexture_;

  // box2d
  b2WorldId world_;
  iforce2d_TopdownCar *car_;

  // HUD
  Timer raceTimer_;
  std::unique_ptr<HUD> HUD_display_;

  // Split screen views
  sf::View leftView_;  // For car 1 (WASD)
  sf::View rightView_; // For car 0 (Arrow keys)

  int redCarIndex_ = -1;  // Which car index is red (uses WASD)
  int blueCarIndex_ = -1; // Which car index is blue (uses Arrow keys)

  // private methods
  sf::Texture *getCarTexture(const std::string &textureId);
  sf::Texture *getObstacleTexture(const std::string &textureId);
};
