#include "RaceScene.hpp"
#include "../System/SceneManager.hpp"
#include "Scenes/MenuScene.hpp"
#include "Scenes/PauseScene.hpp"
#include "Scenes/VictoryScene.hpp"
#include "System/Config.hpp"
#include "System/GameContext.hpp"
#include "System/InputManager.hpp"
#include "box2d/math_functions.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <optional>

const std::string BACKGROUND_IMG{"../assets/backgrounds/RaceScene.png"};
const std::string CAR_IMG{"../assets/vehicles/car.png"};
const std::string OBS_IMG("../assets/obstacles/Obstacle.png");
const std::string SOF_IMG("../assets/obstacles/oil.png");
const std::string CHP_IMG("../assets/others/checkpoint.png");
const std::string BOS_IMG("../assets/obstacles/booster.png");

RaceScene::RaceScene(SceneManager &manager, GameContext &context)
    : context_(context), manager_(manager),
      raceTimer_(context.config->raceDuration),
      HUD_display_(std::make_unique<HUD>(raceTimer_, context_, carProgress_)) {

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
  if (!bosTexture_.loadFromFile(BOS_IMG)) {
    std::cout << "unable to load booster image file" << std::endl;
  }

  carTexture_.setSmooth(true);
  obsTexture_.setSmooth(true);
  chpTexture_.setSmooth(true);
  sofTexture_.setSmooth(true);
  bosTexture_.setSmooth(true);

  // box2D
  b2WorldDef worldDef = b2DefaultWorldDef();
  worldDef.gravity = {0.0f, 0.0f};
  world_ = b2CreateWorld(&worldDef);
  checkpointManager = std::make_unique<CheckpointManager>(
      world_, context.config->numOfLaps, context.config->numOfPlayers);

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
  b2BodyDef topDef = b2DefaultBodyDef();
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

  car_ = iforce2d_TopdownCar::Create(world_, nofCars, nofObstacles_, nofSoft_,
                                     nofBoosters_, startX, startY, spacing);
  std::cout << "test created" << std::endl;
  if (!car_) {
    std::cerr << "error with test creation" << std::endl;
    return;
  }
  std::cout << "nofCars:" << nofCars << "nofObstacles:" << nofObstacles_
            << "nofSoft:" << nofSoft_ << "nofBoosters:" << nofBoosters_
            << std::endl;

  carSprites.resize(nofCars);
  CarStatus_.resize(nofCars);
  for (int i = 0; i < nofCars; i++) {
    carSprites[i].setTexture(carTexture_);

    sf::FloatRect bounds = carSprites[i].getLocalBounds();
    carSprites[i].setOrigin(bounds.width / 2.f, bounds.height / 2.f);

    auto pos = b2Body_GetPosition(car_->GetVehicle(i)->GetBody());
    // std::cout << "box2d Car " << i << " position: " << pos.x << ", " << pos.y
    //         << "\n";
    sf::Vector2f sfPos(pos.x * scale, pos.y * scale);
    carSprites[i].setPosition(sfPos);
  }
  carSprites[1].setColor(sf::Color(255, 0, 0));
  carProgress_.resize(nofCars); // one entry per car

  // obstacles
  obsSprites.resize(nofObstacles_);
  for (int i = 0; i < nofObstacles_; i++) {
    obsSprites[i].setTexture(obsTexture_);
    obsSprites[i].setScale(0.06f, 0.06f);
    sf::FloatRect bounds = obsSprites[i].getLocalBounds();
    obsSprites[i].setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    auto Obpos = b2Body_GetPosition(car_->GetObstacle(i)->GetBody());
    // std::cout << "box2d Obstacle " << " position: " << Obpos.x << ", " <<
    // Obpos.y
    //          << "\n";
    sf::Vector2f sfPos(Obpos.x * scale, Obpos.y * scale);
    obsSprites[i].setPosition(sfPos);
  }

  // soft obstacle
  sofSprites.resize(nofSoft_);
  for (int i = 0; i < nofSoft_; i++) {
    sofSprites[i].setTexture(sofTexture_);
    sofSprites[i].setScale(0.15f, 0.15f);
    sf::FloatRect sofbounds = sofSprites[i].getLocalBounds();
    sofSprites[i].setOrigin(sofbounds.width / 2.f, sofbounds.height / 2.f);
    auto sofpos = b2Body_GetPosition(car_->GetSoftObstacle(i)->GetBody());
    // std::cout << "box2d soft obstacle " << " position: " << sofpos.x << ", "
    //           << sofpos.y << "\n";
    sf::Vector2f sofsfPos(sofpos.x * scale, sofpos.y * scale);
    sofSprites[i].setPosition(sofsfPos);
  }

  // booster
  bosSprites.resize(nofBoosters_);
  for (int i = 0; i < nofBoosters_; i++) {
    bosSprites[i].setTexture(bosTexture_);
    bosSprites[i].setScale(0.15f, 0.15f);
    sf::FloatRect bosbounds = bosSprites[i].getLocalBounds();
    bosSprites[i].setOrigin(bosbounds.width / 2.f, bosbounds.height / 2.f);
    auto bospos = b2Body_GetPosition(car_->GetBooster(i)->GetBody());
    // std::cout << "box2d booster " << " position: " << bospos.x << ", "
    //           << bospos.y << "\n";
    sf::Vector2f bossfPos(bospos.x * scale, bospos.y * scale);
    bosSprites[i].setPosition(bossfPos);
  }

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
    checkpointSprites_[i].setOrigin(chpbounds.width / 2.f,
                                    chpbounds.height / 2.f);

    auto pos = checkpointManager->GetCheckpoints()[i]->GetPosition(); // b2Vec2
    std::cout << "Checkpoint " << i << " position: " << pos.x << ", " << pos.y
              << "\n";

    checkpointSprites_[i].setPosition(
        sf::Vector2f(pos.x * scale, pos.y * scale));
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
    b2SensorBeginTouchEvent *e = sensorEvents.beginEvents + i;
    void *sensorData = b2Shape_GetUserData(e->sensorShapeId);
    void *visitorData = b2Shape_GetUserData(e->visitorShapeId);
    const char *label = static_cast<const char *>(sensorData);
    // Find which car triggered it
    int triggeredCar = -1;
    for (int i = 0; i < nofCars; i++) {
      if (b2Shape_GetUserData(car_->GetVehicle(i)->GetShape()) == visitorData) {
        triggeredCar = i;
        break;
      }
    }
    if (triggeredCar == -1) {
      continue;
    }
    if (label && strcmp(label, "checkpoint") == 0) {
      // Find which checkpoint was triggered
      int triggeredCheckpoint = -1;
      for (size_t idx = 0; idx < checkpointManager->GetCheckpoints().size();
           idx++) {
        if (B2_ID_EQUALS(checkpointManager->GetCheckpoints()[idx]->GetShapeId(),
                         e->sensorShapeId)) {
          triggeredCheckpoint = static_cast<int>(idx);
          break;
        }
      }

      if (triggeredCar != -1 && triggeredCheckpoint != -1) {
        std::cout << "Car " << triggeredCar << " triggered checkpoint "
                  << triggeredCheckpoint << "\n";

        // Update car's checkpoint progress
        CarProgress &progress = carProgress_[triggeredCar];
        if (progress.nextCheckpointIndex == triggeredCheckpoint) {
          progress.nextCheckpointIndex++;
          if (progress.nextCheckpointIndex >=
              checkpointManager->GetCheckpoints().size()) {
            progress.currentLap++;
            progress.nextCheckpointIndex = 0;
            std::cout << "Car " << triggeredCar << " completed lap "
                      << progress.currentLap << "\n";

            if (progress.currentLap >= context_.config->numOfLaps) {
              raceFinished_ = true;
              winnerCarID_ = triggeredCar;
              std::cout << "Car " << winnerCarID_ << " wins the race!\n";
            }
          }
        }
      }
    }
    // soft obstacle
    else if (label && strcmp(label, "softObstacle") == 0) {
      CarStatus_[triggeredCar].inSoftObstacle = true;
      CarStatus_[triggeredCar].timer = 1.0f;
      car_->GetVehicle(triggeredCar)->SetTargetForceStrength(5.0f);
      std::cout << "Car " << triggeredCar << " hit oil!\n";
    }

    else if (label && strcmp(label, "booster") == 0) {
      CarStatus_[triggeredCar].inSoftObstacle = true;
      CarStatus_[triggeredCar].timer = 2.0f;
      car_->GetVehicle(triggeredCar)->SetTargetForceStrength(60.0f);
      car_->GetVehicle(triggeredCar)->SetAccelerationRate(20.0f);
      std::cout << "Car " << triggeredCar << " hit booster!\n";
    }
  }

  // Update vehicles
  for (int i = 0; i < nofCars; i++) {
    if (CarStatus_[i].inSoftObstacle) {
      CarStatus_[i].timer -= dt;
      if (CarStatus_[i].timer <= 0.0f) {
        CarStatus_[i].inSoftObstacle = false;
        car_->GetVehicle(i)->SetTargetForceStrength(30.0f);
        car_->GetVehicle(i)->SetAccelerationRate(5.0f);
        std::cout << "car" << i << "returns to normal!" << std::endl;
      }
    }
    car_->GetVehicle(i)->update(dt);
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

  // Race finished when either timer runs out or a player finishes all laps
  if (raceTimer_.isFinished() || raceFinished_) {
    std::cout << "Race over!\n";
    manager_.pushScene<VictoryScene>(context_, winnerCarID_);
  }
}

void RaceScene::render() {
  // render background (map)
  manager_.getWindow().draw(backgroundSprite_);

  // render obstacles
  for (auto &obstacle : obsSprites) {
    manager_.getWindow().draw(obstacle);
  }

  // render soft obstacles
  for (auto &softObstacle : sofSprites) {
    manager_.getWindow().draw(softObstacle);
  }

  // render boosters
  for (auto &booster : bosSprites) {
    manager_.getWindow().draw(booster);
  }

  // render checkpoints
  for (auto &sprite : checkpointSprites_) {
    manager_.getWindow().draw(sprite);
  }

  // render vehicles
  for (auto &car : carSprites) {
    manager_.getWindow().draw(car);
  }

  // render HUD
  HUD_display_->render(manager_.getWindow());
}
