#include "RaceScene.hpp"
#include "../System/SceneManager.hpp"
#include "Scenes/MenuScene.hpp"
#include "Scenes/PauseScene.hpp"
#include "Scenes/VictoryScene.hpp"
#include "System/GameContext.hpp"
#include "box2d/math_functions.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>

const std::string BACKGROUND_IMG{"../assets/backgrounds/RaceScene.png"};
const std::string CAR_IMG{"../assets/vehicles/car.png"};
const std::string OBS_IMG("../assets/obstacles/Obstacle.png");

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
  if(!obsTexture_.loadFromFile(OBS_IMG)) {
    std::cout << "unable to load obstacles image file" << std::endl;
  }

  carTexture_.setSmooth(true);
  obsTexture_.setSmooth(true);

  // box2D
  b2WorldDef worldDef = b2DefaultWorldDef();
  worldDef.gravity = {0.0f, 0.0f};
  world_ = b2CreateWorld(&worldDef);
  float scale = 30.0f;

  float startX = (window.getSize().x / 2.0f) / scale;
  float startY = (window.getSize().y / 2.0f) / scale;
  float spacing = 3.0f;

  car_ = iforce2d_TopdownCar::Create(world_, nofCars, startX, startY, spacing);

  // b2Vec2 car_pos = car->GetPosition();
  carSprites.resize(nofCars);
  for (int i = 0; i < nofCars; i++) {
    carSprites[i].setTexture(carTexture_);
    
    sf::FloatRect bounds = carSprites[i].getLocalBounds();
    carSprites[i].setOrigin(bounds.width / 2.f, bounds.height / 2.f);

    auto pos = b2Body_GetPosition(car_->GetVehicle(i)->GetBody());
    std::cout << "box2d Car " << i << " position: " << pos.x << ", " << pos.y << "\n";
    sf::Vector2f sfPos(pos.x * scale, pos.y * scale);
    std::cout << "sfml Car " << i << " position: " << sfPos.x << ", " << sfPos.y << "\n";
    carSprites[i].setPosition(sfPos);
  }
  carSprites[1].setColor(sf::Color(255, 0, 0));
  obsSprite_.setTexture(obsTexture_);
  obsSprite_.setScale(0.06f, 0.06f);
  sf::FloatRect bounds = obsSprite_.getLocalBounds();
  obsSprite_.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
  auto Obpos = b2Body_GetPosition(car_->GetObstacle(0)->GetBody());
  std::cout << "box2d Obstacle " << " position: " << Obpos.x << ", " << Obpos.y << "\n";
  sf::Vector2f sfPos(Obpos.x * scale, Obpos.y * scale);
  std::cout << "sfml Obstacle "<< " position: " << sfPos.x << ", " << sfPos.y << "\n";
  obsSprite_.setPosition(sfPos);

  // start timer
  raceTimer_.start();
}


void RaceScene::handleEvent(const sf::Event &event) {
  if (event.type == sf::Event::KeyPressed) {
    switch (event.key.code) {
    case sf::Keyboard::W:
      car_->Keyboard('w', true, 0);
      break;
    case sf::Keyboard::A:
      car_->Keyboard('a', true, 0);
      break;
    case sf::Keyboard::S:
      car_->Keyboard('s', true, 0);
      break;
    case sf::Keyboard::D:
      car_->Keyboard('d', true, 0);
      break;
    case sf::Keyboard::Up:
      car_->Keyboard('w', true, 1);
      break;
    case sf::Keyboard::Left:
      car_->Keyboard('a', true, 1);
      break;
    case sf::Keyboard::Down:
      car_->Keyboard('s', true, 1);
      break;
    case sf::Keyboard::Right:
      car_->Keyboard('d', true, 1);
      break;
    case sf::Keyboard::Escape:
      manager_.pushScene<PauseScene>(context_);
      break;
    }
  } else if (event.type == sf::Event::KeyReleased) {
    switch (event.key.code) {
    case sf::Keyboard::W:
      car_->Keyboard('w', false, 0);
      break;
    case sf::Keyboard::A:
      car_->Keyboard('a', false, 0);
      break;
    case sf::Keyboard::S:
      car_->Keyboard('s', false, 0);
      break;
    case sf::Keyboard::D:
      car_->Keyboard('d', false, 0);
      break;
    case sf::Keyboard::Up:
      car_->Keyboard('w', false, 1);
      break;
    case sf::Keyboard::Left:
      car_->Keyboard('a', false, 1);
      break;
    case sf::Keyboard::Down:
      car_->Keyboard('s', false, 1);
      break;
    case sf::Keyboard::Right:
      car_->Keyboard('d', false, 1);
      break;
    }
  }
}

void RaceScene::update(float dt) {
  b2World_Step(world_, dt, 4);

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

  // render HUD
  HUD_display_->render(manager_.getWindow());
}
