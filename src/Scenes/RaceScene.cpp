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
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <optional>

namespace {
constexpr float PIXELS_PER_METER = 30.0f;
constexpr float DEG2RAD = 0.0174532925199432957f;
const std::string TMX_MAP{"../assets/maps/stas_map.tmx"};
const std::string DEFAULT_CAR_TEXTURE{"../assets/vehicles/car.png"};
} // namespace

RaceScene::RaceScene(SceneManager &manager, GameContext &context)
    : context_(context), manager_(manager),
      raceTimer_(context.config->raceDuration),
      HUD_display_(std::make_unique<HUD>(raceTimer_, context_, carProgress_)),
      useTmxMap_(false) {
  sf::RenderWindow &window = manager_.getWindow();

  // Load TMX map
  if (track_.loadFromFile(TMX_MAP)) {
    useTmxMap_ = true;
    std::cout << "Using TMX map: " << TMX_MAP << std::endl;
  } else {
    useTmxMap_ = false;
    std::cout << "Failed to load TMX map: " << TMX_MAP << std::endl;
    std::cout << "Game will continue but map may not display correctly."
              << std::endl;
  }

  if (!defaultCarTexture_.loadFromFile(DEFAULT_CAR_TEXTURE)) {
    std::cout << "unable to load fallback car image: " << DEFAULT_CAR_TEXTURE
              << std::endl;
  }
  defaultCarTexture_.setSmooth(true);

  const auto &spawnPoints = track_.getVehicleSpawnPoints();
  const auto tileSize = track_.getTileSize();
  const int desiredPlayers = context_.config->numOfPlayers;

  std::cout << "RaceScene: Found " << spawnPoints.size()
            << " spawn points from TMX map" << std::endl;

  if (!spawnPoints.empty()) {
    nofCars = std::min(static_cast<int>(spawnPoints.size()), desiredPlayers);
    std::cout << "Using " << nofCars << " cars from TMX spawn points"
              << std::endl;
    if (desiredPlayers > static_cast<int>(spawnPoints.size())) {
      std::cout << "Warning: Only " << spawnPoints.size()
                << " vehicle spawn point(s) defined in TMX, but "
                << desiredPlayers << " player(s) configured." << std::endl;
    }
  } else {
    nofCars = desiredPlayers;
    std::cout << "No vehicle spawn points found in TMX map. Falling back to "
                 "default positions."
              << std::endl;
  }

  if (nofCars <= 0) {
    nofCars = 1;
  }

  // box2D
  b2WorldDef worldDef = b2DefaultWorldDef();
  worldDef.gravity = {0.0f, 0.0f};
  world_ = b2CreateWorld(&worldDef);

  checkpointManager = std::make_unique<CheckpointManager>(
      world_, context.config->numOfLaps, context.config->numOfPlayers);

  float startX = (window.getSize().x / 2.0f) / PIXELS_PER_METER;
  float startY = (window.getSize().y / 2.0f) / PIXELS_PER_METER;
  float spacing = 3.0f;

  // float worldWidth = window.getSize().x / scale;
  // float worldHeight = window.getSize().y / scale;
  //
  // // left wall
  // b2BodyDef leftDef = b2DefaultBodyDef();
  // leftDef.position = {0.0f, worldHeight / 2.0f};
  // b2BodyId leftWall = b2CreateBody(world_, &leftDef);
  // b2Polygon leftBox = b2MakeBox(0.5f, worldHeight / 2.0f);
  // b2ShapeDef leftShapeDef = b2DefaultShapeDef();
  // b2CreatePolygonShape(leftWall, &leftShapeDef, &leftBox);
  //
  // // right wall
  // b2BodyDef rightDef = b2DefaultBodyDef();
  // rightDef.position = {worldWidth, worldHeight / 2.0f};
  // b2BodyId rightWall = b2CreateBody(world_, &rightDef);
  // b2Polygon rightBox = b2MakeBox(0.5f, worldHeight / 2.0f);
  // b2ShapeDef rightShapeDef = b2DefaultShapeDef();
  // b2CreatePolygonShape(rightWall, &rightShapeDef, &rightBox);
  //
  // // top wall
  // b2BodyDef topDef = b2DefaultBodyDef();
  // topDef.position = {worldWidth / 2.0f, 0.0f};
  // b2BodyId topWall = b2CreateBody(world_, &topDef);
  // b2Polygon topBox = b2MakeBox(worldWidth / 2.0f, 0.5f);
  // b2ShapeDef topShapeDef = b2DefaultShapeDef();
  // b2CreatePolygonShape(topWall, &topShapeDef, &topBox);
  //
  // // bottom wall
  // b2BodyDef botDef = b2DefaultBodyDef();
  // botDef.position = {worldWidth / 2.0f, worldHeight};
  // b2BodyId bottomWall = b2CreateBody(world_, &botDef);
  // b2Polygon bottomBox = b2MakeBox(worldWidth / 2.0f, 0.5f);
  // b2ShapeDef botShapeDef = b2DefaultShapeDef();
  // b2CreatePolygonShape(bottomWall, &botShapeDef, &bottomBox);
  //
  // car_ = iforce2d_TopdownCar::Create(world_, nofCars, nofObstacles_,
  // nofSoft_,
  //                                    nofBoosters_, startX, startY, spacing);
  // std::cout << "test created" << std::endl;
  // if (!car_) {
  //   std::cerr << "error with test creation" << std::endl;
  //   return;
  // }
  // std::cout << "nofCars:" << nofCars << "nofObstacles:" << nofObstacles_
  //           << "nofSoft:" << nofSoft_ << "nofBoosters:" << nofBoosters_
  //           << std::endl;
  //
  // carSprites.resize(nofCars);
  // CarStatus_.resize(nofCars);
  // for (int i = 0; i < nofCars; i++) {
  //   carSprites[i].setTexture(carTexture_);

  if (!spawnPoints.empty()) {
    sf::Vector2f size = spawnPoints[0].size;
    if (size.x <= 0.f || size.y <= 0.f) {
      size = {static_cast<float>(tileSize.x), static_cast<float>(tileSize.y)};
    }
    sf::Vector2f center = spawnPoints[0].position + size * 0.5f;
    startX = center.x / PIXELS_PER_METER;
    startY = center.y / PIXELS_PER_METER;
  }

  car_ = iforce2d_TopdownCar::Create(world_, nofCars, startX, startY, spacing);

  carSprites.clear();
  carSprites.resize(nofCars);

  for (int i = 0; i < nofCars; i++) {
    const Track::SpawnPoint *spawn =
        (!spawnPoints.empty() && i < static_cast<int>(spawnPoints.size()))
            ? &spawnPoints[i]
            : nullptr;

    sf::Texture *texture = &defaultCarTexture_;
    if (spawn) {
      std::cout << "Car " << i << ": tileID=" << spawn->tileID
                << ", textureId='" << spawn->textureId << "'" << std::endl;

      // If object has a tile ID, use that (it will extract from tileset)
      if (spawn->tileID > 0) {
        std::string tileKey = "tile_" + std::to_string(spawn->tileID);
        std::cout << "  Extracting tile texture for GID: " << spawn->tileID
                  << std::endl;
        texture = getCarTexture(tileKey);
      } else if (!spawn->textureId.empty()) {
        // Otherwise use the texture property (blue/red or file path)
        std::cout << "  Using texture property: " << spawn->textureId
                  << std::endl;
        texture = getCarTexture(spawn->textureId);
      } else {
        std::cout << "  No tileID or texture property, using default texture"
                  << std::endl;
      }
    } else {
      std::cout << "Car " << i << ": No spawn point, using default texture"
                << std::endl;
    }
    carSprites[i].setTexture(*texture);

    sf::FloatRect bounds = carSprites[i].getLocalBounds();
    carSprites[i].setOrigin(bounds.width / 2.f, bounds.height / 2.f);

    //   auto pos = b2Body_GetPosition(car_->GetVehicle(i)->GetBody());
    //   // std::cout << "box2d Car " << i << " position: " << pos.x << ", " <<
    //   pos.y
    //   //         << "\n";
    //   sf::Vector2f sfPos(pos.x * scale, pos.y * scale);
    //   carSprites[i].setPosition(sfPos);
    // }
    // carSprites[1].setColor(sf::Color(255, 0, 0));
    // carProgress_.resize(nofCars); // one entry per car
    //
    // // obstacles
    // obsSprites.resize(nofObstacles_);
    // for (int i = 0; i < nofObstacles_; i++) {
    //   obsSprites[i].setTexture(obsTexture_);
    //   obsSprites[i].setScale(0.06f, 0.06f);
    //   sf::FloatRect bounds = obsSprites[i].getLocalBounds();
    //   obsSprites[i].setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    //   auto Obpos = b2Body_GetPosition(car_->GetObstacle(i)->GetBody());
    //   // std::cout << "box2d Obstacle " << " position: " << Obpos.x << ", "
    //   <<
    //   // Obpos.y
    //   //          << "\n";
    //   sf::Vector2f sfPos(Obpos.x * scale, Obpos.y * scale);
    //   obsSprites[i].setPosition(sfPos);
    // }
    //
    // // soft obstacle
    // sofSprites.resize(nofSoft_);
    // for (int i = 0; i < nofSoft_; i++) {
    //   sofSprites[i].setTexture(sofTexture_);
    //   sofSprites[i].setScale(0.15f, 0.15f);
    //   sf::FloatRect sofbounds = sofSprites[i].getLocalBounds();
    //   sofSprites[i].setOrigin(sofbounds.width / 2.f, sofbounds.height / 2.f);
    //   auto sofpos = b2Body_GetPosition(car_->GetSoftObstacle(i)->GetBody());
    //   // std::cout << "box2d soft obstacle " << " position: " << sofpos.x <<
    //   ", "
    //   //           << sofpos.y << "\n";
    //   sf::Vector2f sofsfPos(sofpos.x * scale, sofpos.y * scale);
    //   sofSprites[i].setPosition(sofsfPos);
    // }
    //
    // // booster
    // bosSprites.resize(nofBoosters_);
    // for (int i = 0; i < nofBoosters_; i++) {
    //   bosSprites[i].setTexture(bosTexture_);
    //   bosSprites[i].setScale(0.15f, 0.15f);
    //   sf::FloatRect bosbounds = bosSprites[i].getLocalBounds();
    //   bosSprites[i].setOrigin(bosbounds.width / 2.f, bosbounds.height / 2.f);
    //   auto bospos = b2Body_GetPosition(car_->GetBooster(i)->GetBody());
    //   // std::cout << "box2d booster " << " position: " << bospos.x << ", "
    //   //           << bospos.y << "\n";
    //   sf::Vector2f bossfPos(bospos.x * scale, bospos.y * scale);
    //   bosSprites[i].setPosition(bossfPos);
    // }
    //
    // // add 4 checkpoints
    // checkpointManager->AddCheckpoint(b2Vec2{10.0f, 2.0f});
    // checkpointManager->AddCheckpoint(b2Vec2{2.5f, 2.0f});
    // checkpointManager->AddCheckpoint(b2Vec2{2.5f, 18.0f});
    // checkpointManager->AddCheckpoint(b2Vec2{17.0f, 18.0f});
    // checkpointManager->AddCheckpoint(b2Vec2{26.0f, 18.0f});
    // checkpointManager->AddCheckpoint(b2Vec2{26.0f, 7.0f});
    //
    // // create SFML sprites for each checkpoint
    // checkpointSprites_.resize(checkpointManager->GetCheckpoints().size());
    // for (size_t i = 0; i < checkpointSprites_.size(); i++) {
    //   checkpointSprites_[i].setTexture(chpTexture_);
    //   checkpointSprites_[i].setScale(0.5f, 0.5f);
    //   sf::FloatRect chpbounds = checkpointSprites_[i].getLocalBounds();
    //   checkpointSprites_[i].setOrigin(chpbounds.width / 2.f,
    //                                   chpbounds.height / 2.f);
    //
    //   auto pos = checkpointManager->GetCheckpoints()[i]->GetPosition(); //
    //   b2Vec2 std::cout << "Checkpoint " << i << " position: " << pos.x << ",
    //   " << pos.y
    //             << "\n";
    //
    //   checkpointSprites_[i].setPosition(
    //       sf::Vector2f(pos.x * scale, pos.y * scale));
    // }

    sf::Vector2f targetSize = {static_cast<float>(tileSize.x),
                               static_cast<float>(tileSize.y)};
    if (spawn && (spawn->size.x > 0.f && spawn->size.y > 0.f)) {
      targetSize = spawn->size;
    }

    if (bounds.width > 0.f && bounds.height > 0.f) {
      carSprites[i].setScale(targetSize.x / bounds.width,
                             targetSize.y / bounds.height);
    }

    float rotationDeg = spawn ? spawn->rotation : 0.f;
    float rotationRad = rotationDeg * DEG2RAD;

    b2BodyId body = car_->GetVehicle(i)->GetBody();

    if (spawn) {
      sf::Vector2f center = spawn->position;
      if (spawn->size.x > 0.f && spawn->size.y > 0.f) {
        center += spawn->size * 0.5f;
      } else {
        center += targetSize * 0.5f;
      }

      b2Vec2 position{center.x / PIXELS_PER_METER, center.y / PIXELS_PER_METER};
      b2Rot rotation = b2MakeRot(rotationRad);
      b2Body_SetTransform(body, position, rotation);
      carSprites[i].setPosition(center);
      carSprites[i].setRotation(rotationDeg);
    } else {
      auto pos = b2Body_GetPosition(body);
      carSprites[i].setPosition(
          {pos.x * PIXELS_PER_METER, pos.y * PIXELS_PER_METER});
      carSprites[i].setRotation(rotationDeg);
    }
  }

  // start timer
  raceTimer_.start();

  // Initialize split screen views with zoom
  sf::Vector2u windowSize = window.getSize();
  float viewWidth = windowSize.x / 2.0f;
  float viewHeight = windowSize.y;

  // Zoom factor - smaller value = more zoomed in (shows less area)
  // Adjust this value to change zoom level (0.5 = 2x zoom, 0.25 = 4x zoom,
  // etc.)
  float zoomFactor = 0.4f; // Shows a smaller area around each car

  // Left view (car 0 - WASD) - left half of screen
  leftView_.setViewport(sf::FloatRect(0.0f, 0.0f, 0.5f, 1.0f));
  leftView_.setSize(viewWidth * zoomFactor, viewHeight * zoomFactor);

  // Right view (car 1 - Arrow keys) - right half of screen
  rightView_.setViewport(sf::FloatRect(0.5f, 0.0f, 0.5f, 1.0f));
  rightView_.setSize(viewWidth * zoomFactor, viewHeight * zoomFactor);

  // Set initial centers (will be updated in update() to follow cars)
  if (nofCars > 0) {
    leftView_.setCenter(carSprites[0].getPosition());
  }
  if (nofCars > 1) {
    rightView_.setCenter(carSprites[1].getPosition());
  }
}

void RaceScene::handleEvent(const sf::Event &event) {
  if (event.type == sf::Event::KeyPressed) {
    if (event.key.code == sf::Keyboard::Escape) {
      // =======
      //     switch (event.key.code) {
      //     case sf::Keyboard::W:
      //       car_->Keyboard('w', true, 0);
      //       break;
      //     case sf::Keyboard::A:
      //       car_->Keyboard('a', true, 0);
      //       break;
      //     case sf::Keyboard::S:
      //       car_->Keyboard('s', true, 0);
      //       break;
      //     case sf::Keyboard::D:
      //       car_->Keyboard('d', true, 0);
      //       break;
      //     case sf::Keyboard::Up:
      //       if (nofCars > 1)
      //         car_->Keyboard('w', true, 1);
      //       break;
      //     case sf::Keyboard::Left:
      //       if (nofCars > 1)
      //         car_->Keyboard('a', true, 1);
      //       break;
      //     case sf::Keyboard::Down:
      //       if (nofCars > 1)
      //         car_->Keyboard('s', true, 1);
      //       break;
      //     case sf::Keyboard::Right:
      //       if (nofCars > 1)
      //         car_->Keyboard('d', true, 1);
      //       break;
      //     case sf::Keyboard::Escape:
      // >>>>>>> 2edc667 (Add TMX map support, split-screen views, and physics
      // adjustments)
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
      // =======
      //     switch (event.key.code) {
      //     case sf::Keyboard::W:
      //       car_->Keyboard('w', false, 0);
      //       break;
      //     case sf::Keyboard::A:
      //       car_->Keyboard('a', false, 0);
      //       break;
      //     case sf::Keyboard::S:
      //       car_->Keyboard('s', false, 0);
      //       break;
      //     case sf::Keyboard::D:
      //       car_->Keyboard('d', false, 0);
      //       break;
      //     case sf::Keyboard::Up:
      //       if (nofCars > 1)
      //         car_->Keyboard('w', false, 1);
      //       break;
      //     case sf::Keyboard::Left:
      //       if (nofCars > 1)
      //         car_->Keyboard('a', false, 1);
      //       break;
      //     case sf::Keyboard::Down:
      //       if (nofCars > 1)
      //         car_->Keyboard('s', false, 1);
      //       break;
      //     case sf::Keyboard::Right:
      //       if (nofCars > 1)
      //         car_->Keyboard('d', false, 1);
      //       break;
      // >>>>>>> 2edc667 (Add TMX map support, split-screen views, and physics
      // adjustments)
    }
  }
}

void RaceScene::update(float dt) {
  // Step Box2D world
  b2World_Step(world_, dt, 4);

  // Update track (for animated tiles if any)
  if (useTmxMap_) {
    track_.update(dt);
  }

  float scale = 30.0f;

  // Process sensor events
  //
  // Checkpoint, soft obstacle check
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

  // for (int i = 0; i < nofCars; i++) {
  //   car_->GetVehicle(i)->SetForceStrength(
  //       40.0f); // Reduced from 100.0f for slower speed
  // }

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
    sf::Vector2f sfPos(pos.x * PIXELS_PER_METER, pos.y * PIXELS_PER_METER);
    carSprites[i].setPosition(sfPos);
    carSprites[i].setRotation(angleDeg);

    // Update camera views to follow cars
    if (i == 0) {
      // Left view follows car 0 (WASD)
      leftView_.setCenter(sfPos);
    } else if (i == 1) {
      // Right view follows car 1 (Arrow keys)
      rightView_.setCenter(sfPos);
    }
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
  sf::RenderWindow &window = manager_.getWindow();

  // Clear entire window first
  sf::Color bgColor =
      useTmxMap_ ? track_.getBackgroundColor() : sf::Color::Black;
  window.clear(bgColor);

  // // render obstacles
  // for (auto &obstacle : obsSprites) {
  //   manager_.getWindow().draw(obstacle);
  // }
  //
  // // render soft obstacles
  // for (auto &softObstacle : sofSprites) {
  //   manager_.getWindow().draw(softObstacle);
  // }
  //
  // // render boosters
  // for (auto &booster : bosSprites) {
  //   manager_.getWindow().draw(booster);
  // }
  //
  // // render checkpoints
  // for (auto &sprite : checkpointSprites_) {
  //   manager_.getWindow().draw(sprite);
  // }
  //
  // // render vehicles
  // for (auto &car : carSprites) {
  //   manager_.getWindow().draw(car);
  // }

  // Render left view (car 0 - WASD)
  if (nofCars > 0) {
    window.setView(leftView_);

    // render TMX map
    if (useTmxMap_) {
      window.draw(track_);
    }

    // render all vehicles
    for (int i = 0; i < nofCars; i++) {
      window.draw(carSprites[i]);
    }
  }

  // Render right view (car 1 - Arrow keys)
  if (nofCars > 1) {
    window.setView(rightView_);

    // render TMX map
    if (useTmxMap_) {
      window.draw(track_);
    }

    // render all vehicles
    for (int i = 0; i < nofCars; i++) {
      window.draw(carSprites[i]);
    }
  }

  // Reset to default view for HUD (full screen overlay)
  window.setView(window.getDefaultView());

  // Draw separator line between split screens
  sf::Vector2u windowSize = window.getSize();
  sf::RectangleShape separator(
      sf::Vector2f(2.0f, static_cast<float>(windowSize.y)));
  separator.setPosition(static_cast<float>(windowSize.x) / 2.0f - 1.0f, 0.0f);
  separator.setFillColor(sf::Color::Black);
  window.draw(separator);

  // render HUD on top
  HUD_display_->render(window);
}

sf::Texture *RaceScene::getCarTexture(const std::string &textureId) {
  std::string trimmed = textureId;
  trimmed.erase(trimmed.begin(), std::find_if(trimmed.begin(), trimmed.end(),
                                              [](unsigned char ch) {
                                                return !std::isspace(ch);
                                              }));
  trimmed.erase(std::find_if(trimmed.rbegin(), trimmed.rend(),
                             [](unsigned char ch) { return !std::isspace(ch); })
                    .base(),
                trimmed.end());

  if (trimmed.empty()) {
    return &defaultCarTexture_;
  }

  // Check if this is a tile ID reference (starts with "tile_")
  if (trimmed.substr(0, 5) == "tile_") {
    std::string tileIDStr = trimmed.substr(5);
    try {
      std::uint32_t tileGID = std::stoul(tileIDStr);
      std::string cacheKey = "tile_" + tileIDStr;

      // Check cache first
      auto cached = carTextureCache_.find(cacheKey);
      if (cached != carTextureCache_.end()) {
        return cached->second.get();
      }

      // Extract tile from tileset
      auto extracted = track_.extractTileTexture(tileGID);
      if (extracted) {
        extracted->setSmooth(true);
        sf::Texture *result = extracted.get();
        carTextureCache_.emplace(cacheKey, std::move(extracted));
        std::cout << "Extracted tile texture from tileset for GID: " << tileGID
                  << std::endl;
        return result;
      } else {
        std::cout << "Warning: Could not extract tile " << tileGID
                  << " from tileset." << std::endl;
      }
    } catch (...) {
      std::cout << "Warning: Invalid tile ID format: " << tileIDStr
                << std::endl;
    }
  }

  std::string path = trimmed;
  // Ensure we have a relative path to the vehicles directory if none provided
  if (path.find('/') == std::string::npos &&
      path.find('\\') == std::string::npos) {
    std::string lowered = path;
    std::transform(
        lowered.begin(), lowered.end(), lowered.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    // Replace spaces with underscores for convenience
    std::replace(lowered.begin(), lowered.end(), ' ', '_');
    path = "../assets/vehicles/" + lowered;
  }

  // Append extension if missing
  if (path.find('.') == std::string::npos) {
    path += ".png";
  }

  auto cached = carTextureCache_.find(path);
  if (cached != carTextureCache_.end()) {
    return cached->second.get();
  }

  auto texture = std::make_unique<sf::Texture>();
  if (!texture->loadFromFile(path)) {
    std::cout << "Warning: unable to load car texture '" << textureId
              << "' (attempted path: " << path << "). Using fallback texture."
              << std::endl;
    return &defaultCarTexture_;
  }

  texture->setSmooth(true);
  sf::Texture *result = texture.get();
  carTextureCache_.emplace(path, std::move(texture));
  return result;
}
