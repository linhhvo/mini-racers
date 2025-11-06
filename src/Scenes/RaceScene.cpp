#include "RaceScene.hpp"
#include "../System/SceneManager.hpp"
#include "System/Config.hpp"
#include "Scenes/MenuScene.hpp"
#include "Scenes/PauseScene.hpp"
#include "Scenes/VictoryScene.hpp"
#include "System/GameContext.hpp"
#include "System/InputManager.hpp"
#include "box2d/math_functions.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <cstring>

const std::string BACKGROUND_IMG{"../assets/backgrounds/RaceScene.png"};
const std::string CAR_IMG{"../assets/vehicles/car.png"};
const std::string OBS_IMG("../assets/obstacles/Obstacle.png");
const std::string SOF_IMG("../assets/obstacles/oil.png");
const std::string CHP_IMG("../assets/others/checkpoint.png");

RaceScene::RaceScene(SceneManager &manager, GameContext &context)
    : context_(context), manager_(manager),
      raceTimer_(context.config->raceDuration),
      HUD_display_(std::make_unique<HUD>(raceTimer_, context)) {
  
  sf::RenderWindow &window = manager_.getWindow();

  // apply scene background image
  if (!backgroundTexture_.loadFromFile(BACKGROUND_IMG)) {
    std::cout << "unable to load background image file" << std::endl;
  }

  backgroundTexture_.setSmooth(true);
  backgroundSprite_.setTexture(backgroundTexture_);

  // center background image to the window's center and scale to fit window size
  backgroundSprite_.setOrigin(backgroundSprite_.getLocalBounds().width / 2.f,
                              backgroundSprite_.getLocalBounds().height / 2.f);
  backgroundSprite_.setScale(
      float(window.getSize().x) / backgroundSprite_.getLocalBounds().width,
      float(window.getSize().y) / backgroundSprite_.getLocalBounds().height);
  backgroundSprite_.setPosition(window.getSize().x / 2.f,
                                window.getSize().y / 2.f);

  // apply car image
  if (!carTexture_.loadFromFile(CAR_IMG)) {
    std::cout << "unable to load car image file" << std::endl;
  }
  if (!obsTexture_.loadFromFile(OBS_IMG)) {
    std::cout << "unable to load obstacles image file" << std::endl;
  }
  if (!chpTexture_.loadFromFile(CHP_IMG)) {
    std::cout << "unable to load checkpoint image file" << std::endl;
  }
  if (!sofTexture_.loadFromFile(SOF_IMG)) {
    std::cout << "unable to load oil image file" << std::endl;
  }

  carTexture_.setSmooth(true);
  obsTexture_.setSmooth(true);
  chpTexture_.setSmooth(true);
  sofTexture_.setSmooth(true);

  // box2D
  b2WorldDef worldDef = b2DefaultWorldDef();
  worldDef.gravity = {0.0f, 0.0f};
  world_ = b2CreateWorld(&worldDef);
  checkpointManager = std::make_unique<CheckpointManager>(
    world_, context.config->numOfLaps, context.config->numOfPlayers
  );

  float scale = 30.0f;

  float startX = (window.getSize().x / 2.0f) / scale;
  float startY = (window.getSize().y / 2.0f) / scale;
  float spacing = 3.0f;

  float worldWidth = window.getSize().x / scale;
  float worldHeight = window.getSize().y / scale;

  // left wall
  b2BodyDef leftDef = b2DefaultBodyDef();
  leftDef.position = {0.0f, worldHeight / 2.0f};
  b2BodyId leftWall = b2CreateBody(world_, &leftDef);
  b2Polygon leftBox = b2MakeBox(0.5f, worldHeight / 2.0f);
  b2ShapeDef leftShapeDef = b2DefaultShapeDef();
  b2CreatePolygonShape(leftWall, &leftShapeDef, &leftBox);

  // right wall
  b2BodyDef rightDef = b2DefaultBodyDef();
  rightDef.position = {worldWidth, worldHeight / 2.0f};
  b2BodyId rightWall = b2CreateBody(world_, &rightDef);
  b2Polygon rightBox = b2MakeBox(0.5f, worldHeight / 2.0f);
  b2ShapeDef rightShapeDef = b2DefaultShapeDef();
  b2CreatePolygonShape(rightWall, &rightShapeDef, &rightBox);

  // top wall
  b2BodyDef topDef =  b2DefaultBodyDef();
  topDef.position = {worldWidth / 2.0f, 0.0f};
  b2BodyId topWall = b2CreateBody(world_, &topDef);
  b2Polygon topBox = b2MakeBox(worldWidth / 2.0f, 0.5f);
  b2ShapeDef topShapeDef = b2DefaultShapeDef();
  b2CreatePolygonShape(topWall, &topShapeDef, &topBox);

  // bottom wall
  b2BodyDef botDef = b2DefaultBodyDef();
  botDef.position = {worldWidth / 2.0f, worldHeight};
  b2BodyId bottomWall = b2CreateBody(world_, &botDef);
  b2Polygon bottomBox = b2MakeBox(worldWidth / 2.0f, 0.5f);
  b2ShapeDef botShapeDef = b2DefaultShapeDef();
  b2CreatePolygonShape(bottomWall, &botShapeDef, &bottomBox);

  car_ = iforce2d_TopdownCar::Create(world_, nofCars, startX, startY, spacing);

  carSprites.resize(nofCars);
  for (int i = 0; i < nofCars; i++) {
    carSprites[i].setTexture(carTexture_);

    sf::FloatRect bounds = carSprites[i].getLocalBounds();
    carSprites[i].setOrigin(bounds.width / 2.f, bounds.height / 2.f);

    auto pos = b2Body_GetPosition(car_->GetVehicle(i)->GetBody());
    std::cout << "box2d Car " << i << " position: " << pos.x << ", " << pos.y
              << "\n";
    sf::Vector2f sfPos(pos.x * scale, pos.y * scale);
    std::cout << "sfml Car " << i << " position: " << sfPos.x << ", " << sfPos.y
              << "\n";
    carSprites[i].setPosition(sfPos);
  }
  carSprites[1].setColor(sf::Color(255, 0, 0));
  carProgress_.resize(nofCars); // one entry per car

  // obstacles
  obsSprite_.setTexture(obsTexture_);
  obsSprite_.setScale(0.06f, 0.06f);
  sf::FloatRect bounds = obsSprite_.getLocalBounds();
  obsSprite_.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
  auto Obpos = b2Body_GetPosition(car_->GetObstacle(0)->GetBody());
  std::cout << "box2d Obstacle " << " position: " << Obpos.x << ", " << Obpos.y
            << "\n";
  sf::Vector2f sfPos(Obpos.x * scale, Obpos.y * scale);
  std::cout << "sfml Obstacle " << " position: " << sfPos.x << ", " << sfPos.y
            << "\n";
  obsSprite_.setPosition(sfPos);

  
  // soft obstacle
  sofSprite_.setTexture(sofTexture_);
  sofSprite_.setScale(0.15f, 0.15f);
  sf::FloatRect sofbounds = sofSprite_.getLocalBounds();
  sofSprite_.setOrigin(sofbounds.width / 2.f, sofbounds.height / 2.f);
  auto sofpos = b2Body_GetPosition(car_->GetSoftObstacle(0)->GetBody());
  std::cout << "box2d soft obstacle " << " position: " << sofpos.x << ", "
            << sofpos.y << "\n";
  sf::Vector2f sofsfPos(sofpos.x * scale, sofpos.y * scale);
  std::cout << "sfml soft obstacle " << " position: " << sofsfPos.x << ", "
            << sofsfPos.y << "\n";
  sofSprite_.setPosition(sofsfPos);
  // add 4 checkpoints
  checkpointManager->AddCheckpoint(b2Vec2{10.0f, 2.0f});
  checkpointManager->AddCheckpoint(b2Vec2{2.5f, 2.0f});
  checkpointManager->AddCheckpoint(b2Vec2{2.5f, 18.0f});  
  checkpointManager->AddCheckpoint(b2Vec2{17.0f, 18.0f});
  checkpointManager->AddCheckpoint(b2Vec2{26.0f, 18.0f});
  checkpointManager->AddCheckpoint(b2Vec2{26.0f, 7.0f});
  // create SFML sprites for each checkpoint
  checkpointSprites_.resize(checkpointManager->GetCheckpoints().size());
  for (size_t i = 0; i < checkpointSprites_.size(); i++) {
    checkpointSprites_[i].setTexture(chpTexture_);
    checkpointSprites_[i].setScale(0.5f, 0.5f);
    sf::FloatRect chpbounds = checkpointSprites_[i].getLocalBounds();
    checkpointSprites_[i].setOrigin(chpbounds.width / 2.f, chpbounds.height / 2.f);

    auto pos = checkpointManager->GetCheckpoints()[i]->GetPosition(); // b2Vec2
    std::cout << "Checkpoint " << i << " position: " << pos.x << ", " << pos.y << "\n";

    checkpointSprites_[i].setPosition(sf::Vector2f(pos.x * scale, pos.y * scale));
  }
  // start timer
  raceTimer_.start();
}

void RaceScene::handleEvent(const sf::Event &event) {
  if (event.type == sf::Event::KeyPressed) {
    if (event.key.code == sf::Keyboard::Escape) {
      manager_.pushScene<PauseScene>(context_);
    }

    std::optional<int> playerIdx =
        context_.input->getPlayerFromKey(event.key.code);

    if (playerIdx.has_value()) {
      InputManager::InputAction action =
          context_.input->getPlayerAction(event.key.code, playerIdx.value());
      car_->Keyboard(action, playerIdx.value(), true);
    }

  } else if (event.type == sf::Event::KeyReleased) {
    std::optional<int> playerIdx =
        context_.input->getPlayerFromKey(event.key.code);

    if (playerIdx.has_value()) {
      InputManager::InputAction action =
          context_.input->getPlayerAction(event.key.code, playerIdx.value());
      car_->Keyboard(action, playerIdx.value(), false);
    }
  }
}

void RaceScene::update(float dt) {
    // Step Box2D world
    b2World_Step(world_, dt, 4);

    float scale = 30.0f;

    // Process sensor events
    b2SensorEvents sensorEvents = b2World_GetSensorEvents(world_);
    for (int i = 0; i < sensorEvents.beginCount; i++) {
        b2SensorBeginTouchEvent* e = sensorEvents.beginEvents + i;
        void* sensorData = b2Shape_GetUserData(e->sensorShapeId);
        void* visitorData = b2Shape_GetUserData(e->visitorShapeId);
        const char* label = static_cast<const char*>(sensorData);
        if (label && strcmp(label, "checkpoint") == 0) {
            // Find which checkpoint was triggered
            int triggeredCheckpoint = -1;
            for (size_t idx = 0; idx < checkpointManager->GetCheckpoints().size(); idx++) {
                if (B2_ID_EQUALS(checkpointManager->GetCheckpoints()[idx]->GetShapeId(), e->sensorShapeId)) {
                    triggeredCheckpoint = static_cast<int>(idx);
                    break;
                }
            }

            // Find which car triggered it
            int triggeredCar = -1;
            for (size_t i = 0; i < nofCars; i++) {
                if (b2Shape_GetUserData(car_->GetVehicle(i)->GetShape()) == visitorData) {
                    triggeredCar = static_cast<int>(i);
                    break;
                }
            }

            if (triggeredCar != -1 && triggeredCheckpoint != -1) {
                std::cout << "Car " << triggeredCar 
                          << " triggered checkpoint " << triggeredCheckpoint << "\n";

                // Update car's checkpoint progress
                CarProgress& progress = carProgress_[triggeredCar];
                if (progress.nextCheckpointIndex == triggeredCheckpoint) {
                    progress.nextCheckpointIndex++;
                    if (progress.nextCheckpointIndex >= checkpointManager->GetCheckpoints().size()) {
                        progress.currentLap++;
                        progress.nextCheckpointIndex = 0;
                        std::cout << "Car " << triggeredCar 
                                  << " completed lap " << progress.currentLap << "\n";

                        if (progress.currentLap >= context_.config->numOfLaps) {
                            raceFinished_ = true;
                            winnerCarID_ = triggeredCar;
                            std::cout << "Car " << winnerCarID_ << " wins the race!\n";
                        }
                    }
                }
            }
        }

        if (label && strcmp(label, "softObstacle") == 0) {
            for (int i = 0; i < nofCars; i++) {
                if (b2Shape_GetUserData(car_->GetVehicle(i)->GetShape()) == visitorData) {
                    car_->GetVehicle(i)->SetForceStrength(20.0f);
                    std::cout << "Car " << i << " hit oil!\n";
                }
            }
        }
    }

    // Reset force for all cars
    for (int i = 0; i < nofCars; i++) {
        car_->GetVehicle(i)->SetForceStrength(100.0f);
    }

    // Update vehicles
    for (int i = 0; i < nofCars; i++) {
        car_->GetVehicle(i)->update();
        auto carBody = car_->GetVehicle(i)->GetBody();
        auto transform = b2Body_GetTransform(carBody);
        float angleDeg = b2Rot_GetAngle(transform.q) * RADTODEG;
        auto pos = car_->GetVehicle(i)->GetPosition();
        sf::Vector2f sfPos(pos.x * scale, pos.y * scale);
        carSprites[i].setPosition(sfPos);
        carSprites[i].setRotation(angleDeg);
    }

    // Update HUD and timer
    raceTimer_.update(dt);
    HUD_display_->update();

    // Race finished
    if (raceFinished_) {
        manager_.pushScene<VictoryScene>(context_);
    }

    // Timer finished
    if (raceTimer_.isFinished() && !raceFinished_) {
        std::cout << "Race over!\n";
        manager_.pushScene<VictoryScene>(context_);
    }
}


void RaceScene::render() {
    // render background (map)
    manager_.getWindow().draw(backgroundSprite_);

    // render vehicles
    for (int i = 0; i < nofCars; i++) {
        manager_.getWindow().draw(carSprites[i]);
    }

    // render obstacles
    manager_.getWindow().draw(obsSprite_);
    manager_.getWindow().draw(sofSprite_);

    // render checkpoints
    for (auto &sprite : checkpointSprites_) {
        manager_.getWindow().draw(sprite);
    }

    // render HUD
    HUD_display_->render(manager_.getWindow());
}
