#include "RaceScene.hpp"
#include "../System/SceneManager.hpp"
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
    std::cout << "  Successfully loaded TMX map: " << TMX_MAP << std::endl;
    std::cout << "  Map size: " << track_.getMapSize().x << "x"
              << track_.getMapSize().y << std::endl;
    std::cout << "  Tile size: " << track_.getTileSize().x << "x"
              << track_.getTileSize().y << std::endl;
  } else {
    useTmxMap_ = false;
    std::cerr << "  ERROR: Failed to load TMX map: " << TMX_MAP << std::endl;
    std::cerr << "  Make sure you're running from the project root directory "
                 "(where CMakeLists.txt is)"
              << std::endl;
    std::cerr << "  Or that the path '../assets/maps/stas_map.tmx' is correct "
                 "relative to your working directory."
              << std::endl;
    std::cerr
        << "  Game will continue but screen will be black without the map!"
        << std::endl;
  }

  if (!defaultCarTexture_.loadFromFile(DEFAULT_CAR_TEXTURE)) {
    std::cout << "unable to load fallback car image: " << DEFAULT_CAR_TEXTURE
              << std::endl;
  }
  defaultCarTexture_.setSmooth(true);

  const auto &spawnPoints = track_.getVehicleSpawnPoints();
  const auto &obstacleSpawns = track_.getObstacleSpawnPoints();
  const auto tileSize = track_.getTileSize();
  const int desiredPlayers = context_.config->numOfPlayers;

  std::cout << "RaceScene: Found " << spawnPoints.size()
            << " car spawn points, " << obstacleSpawns.size()
            << "obstacle spawn points from TMX map." << std::endl;

  if (!spawnPoints.empty()) {
    nofCars_ = std::min(static_cast<int>(spawnPoints.size()), desiredPlayers);
    std::cout << "Using " << nofCars_ << " cars from TMX spawn points"
              << std::endl;
    if (desiredPlayers > static_cast<int>(spawnPoints.size())) {
      std::cout << "Warning: Only " << spawnPoints.size()
                << " vehicle spawn point(s) defined in TMX, but "
                << desiredPlayers << " player(s) configured." << std::endl;
    }
  } else {
    nofCars_ = desiredPlayers;
    std::cout << "Warning: No vehicle spawn points found in TMX map. Falling "
                 "back to default positions."
              << std::endl;
    if (!useTmxMap_) {
      std::cerr << "ERROR: TMX map not loaded AND no spawn points = cars may "
                   "not be visible!"
                << std::endl;
    }
  }

  if (nofCars_ <= 0) {
    nofCars_ = 1;
    std::cout << "⚠ Warning: nofCars was 0, setting to 1" << std::endl;
  }

  std::cout << "Final car count: " << nofCars_ << std::endl;

  // obstacle
  nofObstacles_ = obstacleSpawns.size();

  // box2D
  b2WorldDef worldDef = b2DefaultWorldDef();
  worldDef.gravity = {0.0f, 0.0f};
  world_ = b2CreateWorld(&worldDef);

  checkpointManager = std::make_unique<CheckpointManager>(
      world_, context.config->numOfLaps, context.config->numOfPlayers);

  std::vector<b2Vec2> spawnPointsVec;
  std::vector<std::pair<float, float>> sizes;

  // Vehicles
  if (!spawnPoints.empty()) {
    CarStatus_.resize(nofCars_);
    for (int i = 0; i < nofCars_; i++) {
      sf::Vector2f size = spawnPoints[i].size;
      if (size.x <= 0.f || size.y <= 0.f) {
        size = {static_cast<float>(tileSize.x), static_cast<float>(tileSize.y)};
      }
      float halfWidth = (size.x / 2.0f) / PIXELS_PER_METER;
      float halfHeight = (size.y / 2.0f) / PIXELS_PER_METER;
      sizes.push_back(std::pair<float, float>(halfWidth, halfHeight));
      sf::Vector2f center = spawnPoints[0].position + size * 0.5f;
      float startX = center.x / PIXELS_PER_METER;
      float startY = center.y / PIXELS_PER_METER;
      spawnPointsVec.push_back(b2Vec2{startX, startY});
    }
  }
  // Testing car specs
  std::vector<VehicleSpec> specs;
  specs.push_back(VehicleSpec());
  VehicleSpec second{
      .angularDamping = 0.6f,
      .linearDamping = 3.0f,
      .friction = 0.5f,
      .restitution = 0.0f,
      .rollingResistance = 0.0f,
      .brakeRate = 5.0f,
      .baseTargetForceStrength = 10.0f,
      .baseAccelerationRate = 1.0f,
  };
  specs.push_back(second);

  car_ = iforce2d_TopdownCar::Create(world_);
  car_->VehicleFactory(nofCars_, spawnPointsVec, sizes, specs);

  carSprites.clear();
  carSprites.resize(nofCars_);

  for (int i = 0; i < nofCars_; i++) {
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

    // Find which car is red/blue
    if (spawn) {
      std::string textureIdLower = spawn->textureId;
      std::transform(
          textureIdLower.begin(), textureIdLower.end(), textureIdLower.begin(),
          [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
      if (textureIdLower == "red") {
        redCarIndex_ = i;
      } else if (textureIdLower == "blue") {
        blueCarIndex_ = i;
      }
    }
  }

  // obstacles
  if (nofObstacles_ > 0) {
    std::vector<b2Vec2> ObstacleSpawnVec;
    std::vector<std::pair<float, float>> Obstaclesizes;

    for (int i = 0; i < nofObstacles_; i++) {
      sf::Vector2f size = obstacleSpawns[i].size;
      if (size.x <= 0.f || size.y <= 0.f) {
        size = {static_cast<float>(tileSize.x), static_cast<float>(tileSize.y)};
      }
      float halfWidth = (size.x / 2.0f) / PIXELS_PER_METER;
      float halfHeight = (size.y / 2.0f) / PIXELS_PER_METER;
      sizes.push_back(std::pair<float, float>(halfWidth, halfHeight));
      sf::Vector2f center = spawnPoints[0].position + size * 0.5f;
      float startX = center.x / PIXELS_PER_METER;
      float startY = center.y / PIXELS_PER_METER;
      ObstacleSpawnVec.push_back(b2Vec2{startX, startY});
    }

    car_->ObstacleFactory(nofObstacles_, ObstacleSpawnVec, Obstaclesizes);
    obstacleSprites.clear();
    obstacleSprites.resize(nofObstacles_);

    for (int i = 0; i < nofObstacles_; i++) {
      const Track::SpawnPoint *obstacleSpawn = &spawnPoints[i];

      sf::Texture *texture = &defaultObstacleTexture_;
      if (obstacleSpawn) {
        std::cout << "Obstacle " << i << ": tileID=" << obstacleSpawn->tileID
                  << ", textureId='" << obstacleSpawn->textureId << "'"
                  << std::endl;

        // If object has a tile ID, use that (it will extract from tileset)
        if (obstacleSpawn->tileID > 0) {
          std::string tileKey = "tile_" + std::to_string(obstacleSpawn->tileID);
          std::cout << "  Extracting tile texture for GID: "
                    << obstacleSpawn->tileID << std::endl;
          texture = getObstacleTexture(tileKey);
        } else if (!obstacleSpawn->textureId.empty()) {
          // Otherwise use the texture property (blue/red or file path)
          std::cout << "  Using texture property: " << obstacleSpawn->textureId
                    << std::endl;
          texture = getObstacleTexture(obstacleSpawn->textureId);
        } else {
          std::cout << "  No tileID or texture property, using default texture"
                    << std::endl;
        }
      } else {
        std::cout << "Obstacle " << i
                  << ": No spawn point, using default texture" << std::endl;
      }
      obstacleSprites[i].setTexture(*texture);

      sf::FloatRect bounds = obstacleSprites[i].getLocalBounds();
      obstacleSprites[i].setOrigin(bounds.width / 2.f, bounds.height / 2.f);

      sf::Vector2f targetSize = {static_cast<float>(tileSize.x),
                                 static_cast<float>(tileSize.y)};
      if (obstacleSpawn &&
          (obstacleSpawn->size.x > 0.f && obstacleSpawn->size.y > 0.f)) {
        targetSize = obstacleSpawn->size;
      }

      if (bounds.width > 0.f && bounds.height > 0.f) {
        obstacleSprites[i].setScale(targetSize.x / bounds.width,
                                    targetSize.y / bounds.height);
      }

      float rotationDeg = obstacleSpawn ? obstacleSpawn->rotation : 0.f;
      float rotationRad = rotationDeg * DEG2RAD;

      b2BodyId body = car_->GetObstacle(i)->GetBody();

      if (obstacleSpawn) {
        sf::Vector2f center = obstacleSpawn->position;
        if (obstacleSpawn->size.x > 0.f && obstacleSpawn->size.y > 0.f) {
          center += obstacleSpawn->size * 0.5f;
        } else {
          center += targetSize * 0.5f;
        }

        b2Vec2 position{center.x / PIXELS_PER_METER,
                        center.y / PIXELS_PER_METER};
        b2Rot rotation = b2MakeRot(rotationRad);
        b2Body_SetTransform(body, position, rotation);
        obstacleSprites[i].setPosition(center);
        obstacleSprites[i].setRotation(rotationDeg);
      } else {
        auto pos = b2Body_GetPosition(body);
        obstacleSprites[i].setPosition(
            {pos.x * PIXELS_PER_METER, pos.y * PIXELS_PER_METER});
        obstacleSprites[i].setRotation(rotationDeg);
      }
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
  float zoomFactor = 0.5f; // Shows a smaller area around each car

  // Left view (car 1 - WASD) - left half of screen
  leftView_.setViewport(sf::FloatRect(0.0f, 0.0f, 0.5f, 1.0f));
  leftView_.setSize(viewWidth * zoomFactor, viewHeight * zoomFactor);

  // Right view (car 0 - Arrow keys) - right half of screen
  rightView_.setViewport(sf::FloatRect(0.5f, 0.0f, 0.5f, 1.0f));
  rightView_.setSize(viewWidth * zoomFactor, viewHeight * zoomFactor);

  // Set initial centers (will be updated in update() to follow cars)
  if (nofCars_ > 1) {
    sf::Vector2f car1Pos = carSprites[1].getPosition();
    leftView_.setCenter(car1Pos);
    std::cout << "✓ Left view initialized at car 1 position (WASD): ("
              << car1Pos.x << ", " << car1Pos.y << ")" << std::endl;
  }
  if (nofCars_ > 0) {
    sf::Vector2f car0Pos = carSprites[0].getPosition();
    rightView_.setCenter(car0Pos);
    std::cout << "✓ Right view initialized at car 0 position (Arrow keys): ("
              << car0Pos.x << ", " << car0Pos.y << ")" << std::endl;
  }

  std::cout << "RaceScene initialization complete." << std::endl;
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
      // Player 1 (WASD) controls red car, Player 0 (Arrow keys) controls blue
      // car
      int carIndex =
          (playerIdx.value() == 1 && redCarIndex_ >= 0)    ? redCarIndex_
          : (playerIdx.value() == 0 && blueCarIndex_ >= 0) ? blueCarIndex_
                                                           : playerIdx.value();
      car_->Keyboard(action, carIndex, true);
    }

  } else if (event.type == sf::Event::KeyReleased) {
    std::optional<int> playerIdx =
        context_.input->getPlayerFromKey(event.key.code);

    if (playerIdx.has_value()) {
      InputManager::InputAction action =
          context_.input->getPlayerAction(event.key.code, playerIdx.value());
      // Player 1 (WASD) controls red car, Player 0 (Arrow keys) controls blue
      // car
      int carIndex =
          (playerIdx.value() == 1 && redCarIndex_ >= 0)    ? redCarIndex_
          : (playerIdx.value() == 0 && blueCarIndex_ >= 0) ? blueCarIndex_
                                                           : playerIdx.value();
      car_->Keyboard(action, carIndex, false);
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

  // Checkpoint, soft obstacle check
  b2SensorEvents sensorEvents = b2World_GetSensorEvents(world_);
  for (int i = 0; i < sensorEvents.beginCount; i++) {
    b2SensorBeginTouchEvent *e = sensorEvents.beginEvents + i;
    void *sensorData = b2Shape_GetUserData(e->sensorShapeId);
    void *visitorData = b2Shape_GetUserData(e->visitorShapeId);
    const char *label = static_cast<const char *>(sensorData);
    // Find which car triggered it
    int triggeredCar = -1;
    for (int i = 0; i < nofCars_; i++) {
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
    // else if (label && strcmp(label, "softObstacle") == 0) {
    //   CarStatus_[triggeredCar].inSoftObstacle = true;
    //   CarStatus_[triggeredCar].timer = 1.0f;
    //   car_->GetVehicle(triggeredCar)->SetTargetForceStrength(5.0f);
    //   std::cout << "Car " << triggeredCar << " hit oil!\n";
    // }
    //
    // else if (label && strcmp(label, "booster") == 0) {
    //   CarStatus_[triggeredCar].inSoftObstacle = true;
    //   CarStatus_[triggeredCar].timer = 2.0f;
    //   car_->GetVehicle(triggeredCar)->SetTargetForceStrength(60.0f);
    //   car_->GetVehicle(triggeredCar)->SetAccelerationRate(20.0f);
    //   std::cout << "Car " << triggeredCar << " hit booster!\n";
    // }
  }

  for (int i = 0; i < nofCars_; i++) {
    // TODO: Reset car to normal, possibly can be moved to Track?
    if (CarStatus_[i].inSoftObstacle) {
      CarStatus_[i].timer -= dt;
      if (CarStatus_[i].timer <= 0.0f) {
        CarStatus_[i].inSoftObstacle = false;
        car_->GetVehicle(i)->Reset();
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

    // Check checkpoint zones for this car
    if (useTmxMap_) {
      int activatedId = track_.checkAndActivateCheckpoint(sfPos);
      if (activatedId >= 0) {
        // Checkpoint activated - the Track class will handle showing the
        // activated layer
      }
    }

    // Check Soft Obstacle zones for this car
    if (useTmxMap_) {
      int activatedId = track_.checkAndActivateSoft(sfPos);
      if (activatedId >= 0) {
        // Soft obstacle newly activated - apply effect to car
        const auto &softObstacles = track_.getSoftObstacleZones();
        for (const auto &obstacle : softObstacles) {
          if (obstacle.id == activatedId) {
            // Car just entered this soft obstacle - apply effect
            if (!CarStatus_[i].inSoftObstacle) {
              car_->GetVehicle(i)->HitSoftObstacle(
                  obstacle.desiredStrength, obstacle.accelerationStrength);
              CarStatus_[i].inSoftObstacle = true;
              CarStatus_[i].timer = obstacle.activationDuration;
              std::cout << "Car " << i << " hit soft obstacle " << activatedId
                        << " (desiredStrength: " << obstacle.desiredStrength
                        << ", accelerationStrength: "
                        << obstacle.accelerationStrength << ")" << std::endl;
            }
            break;
          }
        }
      }

      // Check if car is still in any soft obstacle (to keep timer reset)
      // We check by seeing if checkAndActivateSoft returns -1 but car is marked
      // as in obstacle If car is still in, the Track will keep the obstacle
      // activated and reset its timer We need to check if car's position is
      // currently in any activated soft obstacle
      if (CarStatus_[i].inSoftObstacle) {
        const auto &softObstacles = track_.getSoftObstacleZones();
        bool stillInObstacle = false;
        for (const auto &obstacle : softObstacles) {
          if (obstacle.activated) {
            // Check if car position is inside this activated obstacle
            bool isInside = (sfPos.x >= obstacle.position.x &&
                             sfPos.x <= obstacle.position.x + obstacle.size.x &&
                             sfPos.y >= obstacle.position.y &&
                             sfPos.y <= obstacle.position.y + obstacle.size.y);
            if (isInside) {
              stillInObstacle = true;
              // Reset timer while car is still in obstacle
              CarStatus_[i].timer = obstacle.activationDuration;
              break;
            }
          }
        }
        // If car left all obstacles, timer will count down in the existing
        // logic below
      }
    }

    // Update camera views to follow cars
    if (i == 0) {
      // Left view follows car 1 (WASD)
      leftView_.setCenter(sfPos);
    } else if (i == 1) {
      // Right view follows car 0 (Arrow keys)
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

  // Debug: Print warning if map not loaded (only once)
  static bool warnedAboutBlackScreen = false;
  if (!useTmxMap_ && !warnedAboutBlackScreen) {
    std::cerr << "⚠ RENDER WARNING: TMX map not loaded - screen will be black!"
              << std::endl;
    std::cerr << "  Check console output above for map loading errors."
              << std::endl;
    warnedAboutBlackScreen = true;
  }

  // Render left view (car 1 - WASD)
  if (nofCars_ > 1) {
    window.setView(leftView_);

    // render TMX map
    if (useTmxMap_) {
      window.draw(track_);
    }

    // render all vehicles
    for (int i = 0; i < nofCars_; i++) {
      window.draw(carSprites[i]);
    }
  }

  // Render right view (car 0 - Arrow keys)
  if (nofCars_ > 0) {
    window.setView(rightView_);

    // render TMX map
    if (useTmxMap_) {
      window.draw(track_);
    }

    // render all vehicles
    for (int i = 0; i < nofCars_; i++) {
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
// TODO: no clue if this works
sf::Texture *RaceScene::getObstacleTexture(const std::string &textureId) {
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
    return &defaultObstacleTexture_;
  }

  // Check if this is a tile ID reference (starts with "tile_")
  if (trimmed.substr(0, 5) == "tile_") {
    std::string tileIDStr = trimmed.substr(5);
    try {
      std::uint32_t tileGID = std::stoul(tileIDStr);
      std::string cacheKey = "tile_" + tileIDStr;

      // Check cache first
      auto cached = obstacleTextureCache_.find(cacheKey);
      if (cached != obstacleTextureCache_.end()) {
        return cached->second.get();
      }

      // Extract tile from tileset
      auto extracted = track_.extractTileTexture(tileGID);
      if (extracted) {
        extracted->setSmooth(true);
        sf::Texture *result = extracted.get();
        obstacleTextureCache_.emplace(cacheKey, std::move(extracted));
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

  auto cached = obstacleTextureCache_.find(path);
  if (cached != obstacleTextureCache_.end()) {
    return cached->second.get();
  }

  auto texture = std::make_unique<sf::Texture>();
  if (!texture->loadFromFile(path)) {
    std::cout << "Warning: unable to load car texture '" << textureId
              << "' (attempted path: " << path << "). Using fallback texture."
              << std::endl;
    return &defaultObstacleTexture_;
  }

  texture->setSmooth(true);
  sf::Texture *result = texture.get();
  obstacleTextureCache_.emplace(path, std::move(texture));
  return result;
}
