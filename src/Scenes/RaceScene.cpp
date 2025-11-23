#include "RaceScene.hpp"
#include "../System/SceneManager.hpp"
#include "Gameplay/GameWorld.hpp"
#include "Gameplay/Obstacles/SoftObstacleManager.hpp"
#include "Race/Track.hpp"
#include "Scenes/PauseScene.hpp"
#include "Scenes/VictoryScene.hpp"
#include "System/Config.hpp"
#include "System/GameContext.hpp"
#include "System/InputManager.hpp"
#include "System/AudioManager.hpp"
#include "box2d/math_functions.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <iostream>
#include <memory>
#include <optional>

namespace {
const std::string DEFAULT_CAR_TEXTURE{"../assets/vehicles/car.png"};
}

RaceScene::RaceScene(SceneManager &manager, GameContext &context)
    : context_(context), manager_(manager),
      raceTimer_(context.config->raceDuration),
      HUD_display_(std::make_unique<HUD>(raceTimer_, context_)), track_(),
      countdown_(3.0f) {

  sf::RenderWindow &window = manager_.getWindow();

  // Load TMX map
  if (track_.loadFromFile(context_.config->TMX_MAP)) {
    useTmxMap_ = true;
    // std::cout << "  Successfully loaded TMX map: " <<
    // context_.config->TMX_MAP
    //           << std::endl;
    // std::cout << "  Map size: " << track_.getMapSize().x << "x"
    //           << track_.getMapSize().y << std::endl;
    // std::cout << "  Tile size: " << track_.getTileSize().x << "x"
    //           << track_.getTileSize().y << std::endl;
  } else {
    useTmxMap_ = false;
    // std::cerr << "  ERROR: Failed to load TMX map: " <<
    // context_.config->TMX_MAP
    //           << std::endl;
    // std::cerr << "  Make sure you're running from the project root directory
    // "
    //              "(where CMakeLists.txt is)"
    //           << std::endl;
    // std::cerr << "  Or that the path '../assets/maps/stas_map.tmx' is correct
    // "
    //              "relative to your working directory."
    //           << std::endl;
    // std::cerr
    //     << "  Game will continue but screen will be black without the map!"
    //     << std::endl;
  }

  if (!defaultCarTexture_.loadFromFile(DEFAULT_CAR_TEXTURE)) {
    std::cout << "unable to load fallback car image: " << DEFAULT_CAR_TEXTURE
              << std::endl;
  }
  defaultCarTexture_.setSmooth(true);

  MapObjectParser parser;
  parser.parse(track_.getTmxMap());

  // parse vehicle, obstacle, checkpoint objects
  const auto &spawnData = parser.getVehicleSpawns();
  const auto &checkpointData = parser.getCheckpoints();
  const auto &softObstacleData = parser.getSoftObstacles();

  // set up box2d world
  b2WorldDef worldDef = b2DefaultWorldDef();
  worldDef.gravity = {0.0f, 0.0f};
  world_ = b2CreateWorld(&worldDef);

  // create container manager for obstacles and checkpoints
  checkpointManager_ = std::make_unique<CheckpointManager>(world_, context_);
  checkpointManager_->loadFromData(checkpointData);

  softObstacleManager_ = std::make_unique<SoftObstacleManager>();
  softObstacleManager_->loadFromData(softObstacleData);

  if (HUD_display_) {
    HUD_display_->setCheckpointManager(checkpointManager_.get());
  }

  std::vector<b2Vec2> spawnPointsVec;
  std::vector<std::pair<float, float>> sizes;

  // Initialize collision tracking
  carCollidedThisFrame_.resize(nofCars_, false);

  // Vehicles
  if (!spawnPoints.empty()) {
    for (int i = 0; i < nofCars_; i++) {
      sf::Vector2f size = spawnPoints[i].size;
      if (size.x <= 0.f || size.y <= 0.f) {
        size = {static_cast<float>(tileSize.x), static_cast<float>(tileSize.y)};
      }
      float halfWidth = (size.x / 2.0f) / context_.config->PIXELS_PER_METER;
      float halfHeight = (size.y / 2.0f) / context_.config->PIXELS_PER_METER;
      sizes.push_back(std::pair<float, float>(halfWidth, halfHeight));
      sf::Vector2f center = spawnPoints[0].position + size * 0.5f;
      float startX = center.x / context_.config->PIXELS_PER_METER;
      float startY = center.y / context_.config->PIXELS_PER_METER;
      spawnPointsVec.push_back(b2Vec2{startX, startY});
    }
  }

  // assign vehicle index to match split screen position
  if (nofCars_ > 0) {
    leftVehicleIndex_ = 0;
  }
  if (nofCars_ > 1) {
    rightVehicleIndex_ = 1;
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
      .baseTargetForceStrength = 3.0f,
      .baseAccelerationRate = 1.0f,
  };

  specs.push_back(second);

  // create vehicle initial spawn points and create box2d world
  std::vector<b2Vec2> spawnPointsVec;
  std::vector<std::pair<float, float>> sizes;

  for (int i = 0; i < nofCars_; i++) {
    const auto &s = spawnData[i];

    sf::Vector2f center = s.position + s.size * 0.5f;

    // convert to Box2D
    float halfW = (s.size.x * 0.5f) / context_.config->PIXELS_PER_METER;
    float halfH = (s.size.y * 0.5f) / context_.config->PIXELS_PER_METER;
    sizes.push_back({halfW, halfH});

    spawnPointsVec.push_back({center.x / context_.config->PIXELS_PER_METER,
                              center.y / context_.config->PIXELS_PER_METER});
  }

  gameWorld_ = GameWorld::Create(world_);
  gameWorld_->VehicleFactory(nofCars_, spawnPointsVec, sizes, specs);

  // set up SFML sprites for vehicles
  carSprites.clear();
  carSprites.resize(nofCars_);

  for (int i = 0; i < nofCars_; i++) {
    const auto &s = spawnData[i];

    sf::Vector2f center = s.position + s.size * 0.5f;
    sf::Texture *vehicleTexture = getCarTexture(s.textureId);
    carSprites[i].setTexture(*vehicleTexture);

    auto bounds = carSprites[i].getLocalBounds();
    carSprites[i].setOrigin(bounds.width / 2, bounds.height / 2);

    carSprites[i].setScale(s.size.x / bounds.width, s.size.y / bounds.height);

    carSprites[i].setPosition(center);
    carSprites[i].setRotation(s.rotation);
  }

  // set up obstacle and vehicle interaction
  softObstacleInteraction_ = std::make_unique<SoftObstacleInteraction>(
      softObstacleManager_.get(), gameWorld_->GetAllVehicles());

  // obstacles
  // if (nofObstacles_ > 0) {
  //   std::vector<b2Vec2> ObstacleSpawnVec;
  //   std::vector<std::pair<float, float>> Obstaclesizes;
  //
  //   for (int i = 0; i < nofObstacles_; i++) {
  //     sf::Vector2f size = obstacleSpawns[i].size;
  //     if (size.x <= 0.f || size.y <= 0.f) {
  //       size = {static_cast<float>(tileSize.x),
  //       static_cast<float>(tileSize.y)};
  //     }
  //     float halfWidth = (size.x / 2.0f) / context_.config->PIXELS_PER_METER;
  //     float halfHeight = (size.y / 2.0f) / context_.config->PIXELS_PER_METER;
  //     sizes.push_back(std::pair<float, float>(halfWidth, halfHeight));
  //     sf::Vector2f center = spawnPoints[0].position + size * 0.5f;
  //     float startX = center.x / context_.config->PIXELS_PER_METER;
  //     float startY = center.y / context_.config->PIXELS_PER_METER;
  //     ObstacleSpawnVec.push_back(b2Vec2{startX, startY});
  //   }
  //
  //   car_->ObstacleFactory(nofObstacles_, ObstacleSpawnVec, Obstaclesizes);
  //   obstacleSprites.clear();
  //   obstacleSprites.resize(nofObstacles_);
  //
  //   for (int i = 0; i < nofObstacles_; i++) {
  //     const Track::SpawnPoint *obstacleSpawn = &spawnPoints[i];
  //
  //     sf::Texture *texture = &defaultObstacleTexture_;
  //     if (obstacleSpawn) {
  //       std::cout << "Obstacle " << i << ": tileID=" << obstacleSpawn->tileID
  //                 << ", textureId='" << obstacleSpawn->textureId << "'"
  //                 << std::endl;
  //
  //       // If object has a tile ID, use that (it will extract from tileset)
  //       if (obstacleSpawn->tileID > 0) {
  //         std::string tileKey = "tile_" +
  //         std::to_string(obstacleSpawn->tileID); std::cout << "  Extracting
  //         tile texture for GID: "
  //                   << obstacleSpawn->tileID << std::endl;
  //         texture = getObstacleTexture(tileKey);
  //       } else if (!obstacleSpawn->textureId.empty()) {
  //         // Otherwise use the texture property (blue/red or file path)
  //         std::cout << "  Using texture property: " <<
  //         obstacleSpawn->textureId
  //                   << std::endl;
  //         texture = getObstacleTexture(obstacleSpawn->textureId);
  //       } else {
  //         std::cout << "  No tileID or texture property, using default
  //         texture"
  //                   << std::endl;
  //       }
  //     } else {
  //       std::cout << "Obstacle " << i
  //                 << ": No spawn point, using default texture" << std::endl;
  //     }
  //     obstacleSprites[i].setTexture(*texture);
  //
  //     sf::FloatRect bounds = obstacleSprites[i].getLocalBounds();
  //     obstacleSprites[i].setOrigin(bounds.width / 2.f, bounds.height / 2.f);
  //
  //     sf::Vector2f targetSize = {static_cast<float>(tileSize.x),
  //                                static_cast<float>(tileSize.y)};
  //     if (obstacleSpawn &&
  //         (obstacleSpawn->size.x > 0.f && obstacleSpawn->size.y > 0.f)) {
  //       targetSize = obstacleSpawn->size;
  //     }
  //
  //     if (bounds.width > 0.f && bounds.height > 0.f) {
  //       obstacleSprites[i].setScale(targetSize.x / bounds.width,
  //                                   targetSize.y / bounds.height);
  //     }
  //
  //     float rotationDeg = obstacleSpawn ? obstacleSpawn->rotation : 0.f;
  //     float rotationRad = rotationDeg * context_.config->DEGTORAD;
  //
  //     b2BodyId body = car_->GetObstacle(i)->GetBody();
  //
  //     if (obstacleSpawn) {
  //       sf::Vector2f center = obstacleSpawn->position;
  //       if (obstacleSpawn->size.x > 0.f && obstacleSpawn->size.y > 0.f) {
  //         center += obstacleSpawn->size * 0.5f;
  //       } else {
  //         center += targetSize * 0.5f;
  //       }
  //
  //       b2Vec2 position{center.x / context_.config->PIXELS_PER_METER,
  //                       center.y / context_.config->PIXELS_PER_METER};
  //       b2Rot rotation = b2MakeRot(rotationRad);
  //       b2Body_SetTransform(body, position, rotation);
  //       obstacleSprites[i].setPosition(center);
  //       obstacleSprites[i].setRotation(rotationDeg);
  //     } else {
  //       auto pos = b2Body_GetPosition(body);
  //       obstacleSprites[i].setPosition(
  //           {pos.x * context_.config->PIXELS_PER_METER,
  //            pos.y * context_.config->PIXELS_PER_METER});
  //       obstacleSprites[i].setRotation(rotationDeg);
  //     }
  //   }
  // }

  // Initialize split screen views with zoom
  sf::Vector2u windowSize = window.getSize();
  float viewWidth = windowSize.x / 2.0f;
  float viewHeight = windowSize.y;

  // Zoom factor - smaller value = more zoomed in (shows less area)
  // Adjust this value to change zoom level (0.5 = 2x zoom, 0.25 = 4x zoom,
  // etc.)
  float zoomFactor = 0.5f;

  // Left view (car 1 - WASD) - left half of screen
  leftView_.setViewport(sf::FloatRect(0.0f, 0.0f, 0.5f, 1.0f));
  leftView_.setSize(viewWidth * zoomFactor, viewHeight * zoomFactor);

  // Right view (car 0 - Arrow keys) - right half of screen
  rightView_.setViewport(sf::FloatRect(0.5f, 0.0f, 0.5f, 1.0f));
  rightView_.setSize(viewWidth * zoomFactor, viewHeight * zoomFactor);

  if (leftVehicleIndex_ >= 0) {
    leftView_.setCenter(carSprites[leftVehicleIndex_].getPosition());
  }
  if (rightVehicleIndex_ >= 0) {
    rightView_.setCenter(carSprites[rightVehicleIndex_].getPosition());
  }

  std::cout << "RaceScene initialization complete." << std::endl;
}

RaceScene::~RaceScene() {
  if (b2World_IsValid(world_)) {
    b2DestroyWorld(world_);
  }
}

void RaceScene::handleEvent(const sf::Event &event) {
  if (event.type == sf::Event::KeyPressed) {
    if (event.key.code == sf::Keyboard::Escape) {
      manager_.pushScene<PauseScene>(context_);
      return;
    }

    if (countdown_.isBlockingInput())
      return;

    std::optional<int> playerIdx =
        context_.input->getPlayerFromKey(event.key.code);

    if (playerIdx.has_value()) {
      InputManager::InputAction action =
          context_.input->getPlayerAction(event.key.code, playerIdx.value());
      // left view player uses WASD for vehicle control
      int carIndex = (playerIdx.value() == 0 && leftVehicleIndex_ >= 0)
                         ? leftVehicleIndex_
                     : (playerIdx.value() == 1 && rightVehicleIndex_ >= 0)
                         ? rightVehicleIndex_
                         : playerIdx.value();
      gameWorld_->Keyboard(action, carIndex, true);
    }

  } else if (event.type == sf::Event::KeyReleased) {
    if (countdown_.isBlockingInput())
      return;
    std::optional<int> playerIdx =
        context_.input->getPlayerFromKey(event.key.code);

    if (playerIdx.has_value()) {
      InputManager::InputAction action =
          context_.input->getPlayerAction(event.key.code, playerIdx.value());
      // right view player uses arrow keys for vehicle control
      int carIndex = (playerIdx.value() == 0 && leftVehicleIndex_ >= 0)
                         ? leftVehicleIndex_
                     : (playerIdx.value() == 1 && rightVehicleIndex_ >= 0)
                         ? rightVehicleIndex_
                         : playerIdx.value();
      gameWorld_->Keyboard(action, carIndex, false);
    }
  }
}

void RaceScene::update(float dt) {
  // Step Box2D world
  if (!countdown_.isBlockingInput()) {
    b2World_Step(world_, dt, 4);
  }

  checkpointManager_->update(dt);
  softObstacleInteraction_->update(dt);
  softObstacleManager_->update(dt);

  // Reset collision tracking for this frame
  for (int i = 0; i < nofCars_; i++) {
    carCollidedThisFrame_[i] = false;
  }

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
      if (b2Shape_GetUserData(gameWorld_->GetVehicle(i)->GetShape()) ==
          visitorData) {
        triggeredCar = i;
        break;
      }
    }
    if (triggeredCar == -1) {
      continue;
    }
    if (label && strcmp(label, "checkpoint") == 0) {
      // Find which checkpoint was triggered
      Checkpoint *cp =
          checkpointManager_->FindCheckpointByShapeId(e->sensorShapeId);

      if (cp) {
        checkpointManager_->onCarHitCheckpoint(triggeredCar, cp);
        AudioManager::Instance().PlaySound("../assets/sounds/audio/checkpoint_sound.wav");
        std::cout << "[Audio] Checkpoint sound played" << std::endl;
      }
      // race finish
      if (checkpointManager_->isRaceFinished()) {
        raceFinished_ = true;
        winnerCarID_ = checkpointManager_->getWinnerCarID();
      }
    }
  }

  // Detect vehicle-to-vehicle collisions for sound effects
  b2ContactEvents contactEvents = b2World_GetContactEvents(world_);
  for (int i = 0; i < contactEvents.beginCount; i++) {
    b2ContactBeginTouchEvent *e = contactEvents.beginEvents + i;
    // Check if a car is involved in the collision
    for (int j = 0; j < nofCars_; j++) {
      if (b2Shape_GetUserData(car_->GetVehicle(j)->GetShape()) == 
          b2Shape_GetUserData(e->shapeIdA) ||
          b2Shape_GetUserData(car_->GetVehicle(j)->GetShape()) == 
          b2Shape_GetUserData(e->shapeIdB)) {
        AudioManager::Instance().SetSoundVolume(100.0f);
        AudioManager::Instance().PlaySound("../assets/sounds/audio/collision_sound.wav");
        AudioManager::Instance().SetSoundVolume(70.0f);  // Reset to default
        std::cout << "[Audio] Vehicle collision detected at 100%" << std::endl;
        break;
      }
    }
  }

  for (int i = 0; i < nofCars_; i++) {
    gameWorld_->GetVehicle(i)->update(dt);

    auto pos = gameWorld_->GetVehicle(i)->GetPosition();
    sf::Vector2f sfPos(pos.x * context_.config->PIXELS_PER_METER,
                       pos.y * context_.config->PIXELS_PER_METER);

    auto carBody = gameWorld_->GetVehicle(i)->GetBody();
    float angleDeg = b2Rot_GetAngle(b2Body_GetTransform(carBody).q) *
                     context_.config->RADTODEG;

    carSprites[i].setPosition(sfPos);
    carSprites[i].setRotation(angleDeg);

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
              
              // Play sound based on obstacle type
              if (obstacle.desiredStrength > 0.5f) {
                // Booster detected (high desiredStrength)
                AudioManager::Instance().PlaySound("../assets/sounds/audio/booster_sound.wav");
                std::cout << "[Audio] Booster sound played" << std::endl;
              } else {
                // Oil/soft obstacle detected
                AudioManager::Instance().SetSoundVolume(100.0f);
                AudioManager::Instance().PlaySound("../assets/sounds/audio/collision_sound.wav");
                AudioManager::Instance().SetSoundVolume(70.0f);  // Reset to default
                std::cout << "[Audio] Soft obstacle sound played at 100%" << std::endl;
              }
              
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
    if (i == leftVehicleIndex_) {
      // Left view follows car 1 (WASD)
      leftView_.setCenter(sfPos);
    } else if (i == rightVehicleIndex_) {
      // Right view follows car 0 (Arrow keys)
      rightView_.setCenter(sfPos);
    }
  }

  // Update HUD and timer
  raceTimer_.update(dt);
  HUD_display_->update();

  // Race start countdown
  if (countdown_.update(dt)) {
    raceTimer_.start();
    // Start race music after "GO!" sound finishes playing
    AudioManager::Instance().SetMusicVolume(40.0f);  // Set race music to 40%
    AudioManager::Instance().PlayMusic("../assets/sounds/music/race_music.wav");
    std::cout << "[Audio] Race music started at 40%" << std::endl;
  }

  // Race finished when either timer runs out or a player finishes all laps
  if (raceTimer_.isFinished() || raceFinished_) {
    if (victoryDelayTimer_ < 0.f) {
      victoryDelayTimer_ = 0.8f; // delay before showing victory scene
      AudioManager::Instance().PlaySound("../assets/sounds/audio/end_game_sound.wav");
      AudioManager::Instance().StopMusic();  // Stop race music when game ends
      std::cout << "[Audio] Race ended, music stopped" << std::endl;
    } else {
      victoryDelayTimer_ -= dt;
      if (victoryDelayTimer_ <= 0.f) {
        std::cout << "Race over!\n";
        manager_.pushScene<VictoryScene>(context_, winnerCarID_);
        return;
      }
    }
  }

  // render activated checkpoint in Track class
  track_.setActiveCheckpointLayers(
      checkpointManager_->getActivatedCheckpointID());
}

void RaceScene::render() {
  sf::RenderWindow &window = manager_.getWindow();

  // Clear entire window first
  window.clear(track_.getBackgroundColor());

  // Render left view (car 1 - WASD)
  if (nofCars_ > 1) {
    window.setView(leftView_);
    if (useTmxMap_) {
      window.draw(track_);
    }
    for (auto &vehicle : carSprites) {
      window.draw(vehicle);
    }
  }

  // Render right view (car 0 - Arrow keys)
  if (nofCars_ > 0) {
    window.setView(rightView_);
    if (useTmxMap_) {
      window.draw(track_);
    }
    for (auto &vehicle : carSprites) {
      window.draw(vehicle);
    }
  }

  // Reset to default view for HUD (full screen overlay)
  window.setView(window.getDefaultView());

  // Draw separator line between split screens
  sf::RectangleShape separator(
      sf::Vector2f(2.0f, static_cast<float>(window.getSize().y)));
  separator.setPosition(static_cast<float>(window.getSize().x) / 2.0f - 1.0f,
                        0.0f);
  separator.setFillColor(sf::Color::Black);
  window.draw(separator);

  // Countdown overlay
  countdown_.render(window);

  // render HUD on top
  HUD_display_->render(window);
}

sf::Texture *RaceScene::getCarTexture(const std::string &id) {
  if (id.empty()) {
    return &defaultCarTexture_;
  }

  std::string key = id;
  std::transform(key.begin(), key.end(), key.begin(), ::tolower);

  // extract tile id from tileset
  if (key.rfind("tile_", 0) == 0) {
    std::string gidStr = key.substr(5);
    uint32_t gid = std::stoul(gidStr);

    if (auto it = carTextureCache_.find(key); it != carTextureCache_.end())
      return it->second.get();

    auto texture = track_.extractTileTexture(gid);
    if (texture) {
      texture->setSmooth(true);
      auto *texturePtr = texture.get();
      carTextureCache_[key] = std::move(texture);
      return texturePtr;
    }
    return &defaultCarTexture_;
  }

  // Load texture from file in vehicles folder
  std::string path = id;
  if (path.find('.') == std::string::npos)
    path += ".png";

  if (path.find('/') == std::string::npos)
    path = "../assets/vehicles/" + path;

  if (auto it = carTextureCache_.find(path); it != carTextureCache_.end())
    return it->second.get();

  auto texture = std::make_unique<sf::Texture>();
  if (!texture->loadFromFile(path))
    return &defaultCarTexture_;

  texture->setSmooth(true);
  auto *texturePtr = texture.get();
  carTextureCache_[path] = std::move(texture);
  return texturePtr;
}
