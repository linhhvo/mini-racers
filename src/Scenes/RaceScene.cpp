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

  // checkpoint
  // chpSprite_.setTexture(chpTexture_);
  // chpSprite_.setScale(0.5f, 0.5f);
  // sf::FloatRect chpbounds = chpSprite_.getLocalBounds();
  // chpSprite_.setOrigin(chpbounds.width / 2.f, chpbounds.height / 2.f);
  // auto chppos = b2Body_GetPosition(car_->GetCheckPoint()->GetBody());
  // std::cout << "box2d checkpoint " << " position: " << chppos.x << ", "
  //           << chppos.y << "\n";
  // sf::Vector2f chpsfPos(chppos.x * scale, chppos.y * scale);
  // std::cout << "sfml checkpoint " << " position: " << chpsfPos.x << ", "
  //           << chpsfPos.y << "\n";
  // chpSprite_.setPosition(chpsfPos);

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
  checkpointManager->AddCheckpoint(b2Vec2{13.3333f, 10.0f});
  checkpointManager->AddCheckpoint(b2Vec2{16.3333, 10.0f});
  checkpointManager->AddCheckpoint(b2Vec2{15.0f, 5.0f});  
  checkpointManager->AddCheckpoint(b2Vec2{12.0f, 12.0f});
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
  b2World_Step(world_, dt, 4);
  // Checkpoint, soft obstacle check
  b2SensorEvents sensorEvents = b2World_GetSensorEvents(world_);
  for (int i = 0; i < sensorEvents.beginCount; i++) {
    b2SensorBeginTouchEvent *e = sensorEvents.beginEvents + i;
    void *sensorData = b2Shape_GetUserData(e->sensorShapeId);
    void *visitorData = b2Shape_GetUserData(e->visitorShapeId);
    if (sensorData == (void *)"checkpoint") {
      std::cout << "Checkpoint triggered!\n";
      checkpointManager->HandleSensorEvent(e);
    }
    if (sensorData == (void *)"softObstacle") {
      for (int i = 0; i < nofCars; i++) {
        auto data = b2Shape_GetUserData(car_->GetVehicle(i)->GetShape());
        if (data == visitorData) {
          car_->GetVehicle(i)->SetForceStrength(20.0f);
          std::cout << "oil hit!\n";
        }
      }
    }
  }
  for (int i = 0; i < nofCars; i++) {
    car_->GetVehicle(i)->SetForceStrength(100.0f);
  }

  for (int i = 0; i < nofCars; i++) {
    float scale = 30.0f;
    car_->GetVehicle(i)->update();
    auto carBody = car_->GetVehicle(i)->GetBody();
    auto transform = b2Body_GetTransform(carBody);
    float angleDeg = b2Rot_GetAngle(transform.q) * RADTODEG;
    auto pos = car_->GetVehicle(i)->GetPosition();
    sf::Vector2f sfPos(pos.x * scale, pos.y * scale);
    carSprites[i].setPosition(sfPos);

    carSprites[i].setRotation(angleDeg);
  }

  raceTimer_.update(dt);
  HUD_display_->update();
  if (checkpointManager->IsRaceFinished()) {
    int winner = checkpointManager->GetWinner();
    std::cout << "Car " << winner << " wins the race!\n";
    manager_.pushScene<VictoryScene>(context_);
  }
  if (raceTimer_.isFinished()) {
    std::cout << "race over" << std::endl;
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

  manager_.getWindow().draw(obsSprite_);
  manager_.getWindow().draw(chpSprite_);
  manager_.getWindow().draw(sofSprite_);

  // render HUD
  HUD_display_->render(manager_.getWindow());
}
