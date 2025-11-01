#include "RaceScene.hpp"
#include "../System/SceneManager.hpp"
#include "Scenes/MenuScene.hpp"
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

  carTexture_.setSmooth(true);

  // box2D
  b2WorldDef worldDef = b2DefaultWorldDef();
  worldDef.gravity = {0.0f, 0.0f};
  world_ = b2CreateWorld(&worldDef);

  car_ = iforce2d_TopdownCar::Create(world_, nofCars);

  // b2Vec2 car_pos = car->GetPosition();
  carSprites.resize(nofCars);
  for (int i = 0; i < nofCars; i++) {
    carSprites[i].setTexture(carTexture_);
    carSprites[i].setPosition(window.getSize().x / 2.f + spacing * i,
                              window.getSize().y / 2.f + spacing * i);
  }

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
    car_->GetVehicle(i)->update();
    auto carBody = car_->GetVehicle(i)->GetBody();
    auto transform = b2Body_GetTransform(carBody);
    float angleDeg = b2Rot_GetAngle(transform.q) * RADTODEG;
    carSprites[i].setPosition(400 + car_->GetVehicle(i)->GetPosition().x,
                              300 + car_->GetVehicle(i)->GetPosition().y);
    carSprites[i].setRotation(angleDeg);
  }

  raceTimer_.update();
  HUD_display_->update();

  if (raceTimer_.isFinished()) {
    std::cout << "race over" << std::endl;
    exit(1);
  }
}

void RaceScene::render() {
  // render background (map)
  manager_.getWindow().draw(backgroundSprite_);

  // render vehicles
  for (int i = 0; i < nofCars; i++) {
    manager_.getWindow().draw(carSprites[i]);
  }

  // render HUD
  HUD_display_->render(manager_.getWindow());
}
