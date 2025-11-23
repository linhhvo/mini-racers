#pragma once
#include "../System/SceneManager.hpp"
#include "Gameplay/Checkpoints/CheckpointManager.hpp"
#include "Gameplay/GameWorld.hpp"
#include "Gameplay/Obstacles/SoftObstacleInteraction.hpp"
#include "Gameplay/Obstacles/SoftObstacleManager.hpp"
#include "Race/Countdown.hpp"
#include "Race/HUD.hpp"
#include "Race/Timer.hpp"
#include "Race/Track.hpp"
#include "Scene.hpp"
#include "System/GameContext.hpp"
#include "box2d/id.h"
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
  RaceScene(SceneManager &manager, GameContext &context);
  ~RaceScene();

  void handleEvent(const sf::Event &event);
  void update(float dt);
  void render();

private:
  // counters
  int nofCars_ = 0;
  int nofObstacles_ = 0;

  // system
  InputManager input_;
  GameContext &context_;

  // checkpoints and win condition
  std::unique_ptr<CheckpointManager> checkpointManager_;
  bool raceFinished_ = false;
  int winnerCarID_ = -1;
  float victoryDelayTimer_ = -1.f;
  CountdownOverlay countdown_;

  // map
  Track track_;
  bool useTmxMap_ = false;

  // scene
  SceneManager &manager_;

  // vehicles
  std::vector<sf::Sprite> carSprites;
  std::unordered_map<std::string, std::unique_ptr<sf::Texture>>
      carTextureCache_;
  sf::Texture defaultCarTexture_;
  std::vector<bool> carCollidedThisFrame_; ///< Track collisions this frame

  // obstacles
  std::unique_ptr<SoftObstacleManager> softObstacleManager_;
  std::unique_ptr<SoftObstacleInteraction> softObstacleInteraction_;

  // box2d
  b2WorldId world_{b2_nullWorldId};
  std::unique_ptr<GameWorld> gameWorld_;

  // HUD
  Timer raceTimer_;
  std::unique_ptr<HUD> HUD_display_;

  // Split screen views
  sf::View leftView_;  // For car 1 (WASD)
  sf::View rightView_; // For car 0 (Arrow keys)

  int leftVehicleIndex_ = -1;  // Which car index is red (uses WASD)
  int rightVehicleIndex_ = -1; // Which car index is blue (uses Arrow keys)

  // private methods
  sf::Texture *getCarTexture(const std::string &textureId);
};
