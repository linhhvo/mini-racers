#include "RaceScene.hpp"
#include "../System/SceneManager.hpp"
#include "Gameplay/GameWorld.hpp"
#include "Gameplay/Obstacles/SoftObstacleManager.hpp"
#include "Race/Track.hpp"
#include "Scenes/PauseScene.hpp"
#include "Scenes/VictoryScene.hpp"
#include "System/AudioManager.hpp"
#include "System/Config.hpp"
#include "System/GameContext.hpp"
#include "System/InputManager.hpp"
#include "box2d/math_functions.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <algorithm>
#include <cctype>
#include <cmath>
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
  } else {
    useTmxMap_ = false;
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

  // create map border obstacles from tile layer (static collision bodies)
  mapBorders_.clear();
  const auto &borderTiles = track_.getBorderTiles();

  for (const auto &borderTile : borderTiles) {
    // Convert to Box2D coordinates (pixels to meters)
    b2Vec2 worldPos(borderTile.position.x / context_.config->PIXELS_PER_METER,
                    borderTile.position.y / context_.config->PIXELS_PER_METER);

    // Convert size to Box2D (half-width and half-height for b2MakeBox)
    // Obstacle constructor takes (height, width) and b2MakeBox expects
    // half-extents
    float halfWidth =
        (borderTile.size.x * 0.5f) / context_.config->PIXELS_PER_METER;
    float halfHeight =
        (borderTile.size.y * 0.5f) / context_.config->PIXELS_PER_METER;

    // Create static obstacle body
    // Obstacle constructor: (world, position, height, width)
    mapBorders_.push_back(
        std::make_unique<Obstacle>(world_, worldPos, halfHeight, halfWidth));
  }

  if (HUD_display_) {
    HUD_display_->setCheckpointManager(checkpointManager_.get());
  }

  // determine vehicle count
  const int desiredPlayers = context_.config->numOfPlayers;
  nofCars_ = std::min((int)spawnData.size(), desiredPlayers);
  if (nofCars_ < 1) {
    nofCars_ = 1;
  }

  // set up split views
  camera_.configure(nofCars_, window.getSize());
  camera_.setZoom(0.5f);

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

  // Checkpoint sensor detection
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
      if (b2Shape_GetUserData(gameWorld_->GetVehicle(j)->GetShape()) ==
              b2Shape_GetUserData(e->shapeIdA) ||
          b2Shape_GetUserData(gameWorld_->GetVehicle(j)->GetShape()) ==
              b2Shape_GetUserData(e->shapeIdB)) {
        AudioManager::Instance().SetSoundVolume(100.0f);
        AudioManager::Instance().PlaySound(
            "../assets/sounds/audio/collision_sound.wav");
        AudioManager::Instance().SetSoundVolume(70.0f); // Reset to default
        break;
      }
    }
  }

  // update vehicle movement and sprite transform
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

    softObstacleInteraction_->onCarPositionUpdated(i, sfPos);

    // Off-track speed reduction
    Vehicle *vehicle = gameWorld_->GetVehicle(i);
    float baseSpeed = vehicle->GetBaseTargetForceStrength();
    float currentSpeed = vehicle->GetTargetForceStrength();
    bool isOnTrack = track_.isPositionOnTrack(sfPos);

    // Off-track speed multiplier (60% of base speed)
    const float offTrackSpeedMultiplier = 0.6f;
    float offTrackSpeed = baseSpeed * offTrackSpeedMultiplier;

    // Tolerance for comparing speeds (to account for floating point precision)
    const float speedTolerance = 0.01f;

    // Check if car is at base speed (not affected by soft obstacles)
    bool isAtBaseSpeed = std::abs(currentSpeed - baseSpeed) < speedTolerance;
    // Check if car is at off-track speed
    bool isAtOffTrackSpeed =
        std::abs(currentSpeed - offTrackSpeed) < speedTolerance;

    if (isOnTrack) {
      // On track: restore base speed if currently at off-track speed
      if (isAtOffTrackSpeed) {
        vehicle->SetTargetForceStrength(baseSpeed);
      }
    } else {
      // Off track: apply speed reduction only if at base speed (not affected by
      // soft obstacles)
      if (isAtBaseSpeed) {
        vehicle->SetTargetForceStrength(offTrackSpeed);
      }
    }

    // Update camera views to follow cars
    camera_.setCarTarget(i, sfPos);
  }

  camera_.updateViews();

  // Update HUD and timer
  raceTimer_.update(dt);
  HUD_display_->update();

  // Race start countdown
  if (countdown_.update(dt)) {
    raceTimer_.start();
    // Start race music after "GO!" sound finishes playing
    AudioManager::Instance().SetMusicVolume(40.0f); // Set race music to 40%
    AudioManager::Instance().PlayMusic("../assets/sounds/music/race_music.wav");
  }

  // Race finished when either timer runs out or a player finishes all laps
  if (raceTimer_.isFinished() || raceFinished_) {
    if (victoryDelayTimer_ < 0.f) {
      victoryDelayTimer_ = 0.8f; // delay before showing victory scene
      AudioManager::Instance().PlaySound(
          "../assets/sounds/audio/end_game_sound.wav");
      AudioManager::Instance().StopMusic(); // Stop race music when game ends
    } else {
      victoryDelayTimer_ -= dt;
      if (victoryDelayTimer_ <= 0.f) {
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

  for (int i = 0; i < camera_.getNumViews(); i++) {
    window.setView(camera_.getView(i));

    if (useTmxMap_)
      window.draw(track_);

    for (auto &sprite : carSprites)
      window.draw(sprite);
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
