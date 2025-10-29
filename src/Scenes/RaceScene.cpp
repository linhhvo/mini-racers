#include "RaceScene.hpp"
#include "Scenes/MenuScene.hpp"
#include "Scenes/SceneManager.hpp"
#include "System/InputManager.hpp"
#include "box2d/math_functions.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <iostream>

const std::string BACKGROUND_IMG{"../assets/backgrounds/RaceScene.png"};
const std::string CAR_IMG{"../assets/vehicles/car.png"};

RaceScene::RaceScene(SceneManager &manager) : manager_(manager) {
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
  carSprite_.setTexture(carTexture_);

  // box2D
  b2WorldDef worldDef = b2DefaultWorldDef();
  worldDef.gravity = {0.0f, 0.0f};
  world_ = b2CreateWorld(&worldDef);

  car_ = iforce2d_TopdownCar::Create(world_);
  Vehicle *car = car_->GetVehicle();

  // b2Vec2 car_pos = car->GetPosition();
  carSprite_.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);


  // std::cout << "pos of vehicle body at x = " << carSprite_.getPosition().x
  //           << " and y = " << carSprite_.getPosition().y << std::endl;
}

void RaceScene::handleEvent(const sf::Event &event) {
  if (event.type == sf::Event::KeyPressed) {
    switch(event.key.code) {
      case sf::Keyboard::W: car_->Keyboard('w', true); break;
      case sf::Keyboard::A: car_->Keyboard('a', true); break;
      case sf::Keyboard::S: car_->Keyboard('s', true); break;
      case sf::Keyboard::D: car_->Keyboard('d', true); break;
    }
  }
  else if (event.type == sf::Event::KeyReleased) {
    switch(event.key.code) {
      case sf::Keyboard::W: car_->Keyboard('w', false); break;
      case sf::Keyboard::A: car_->Keyboard('a', false); break;
      case sf::Keyboard::S: car_->Keyboard('s', false); break;
      case sf::Keyboard::D: car_->Keyboard('d', false); break;
    }
  }
}

void RaceScene::update(float dt) {
  b2World_Step(world_, dt, 4);
  car_->GetVehicle()->update();
  auto carBody = car_->GetVehicle()->GetBody();
  auto transform = b2Body_GetTransform(carBody);
  float angleDeg = b2Rot_GetAngle(transform.q) * RADTODEG;
  carSprite_.setPosition(400 + car_->GetVehicle()->GetPosition().x,
                         300 + car_->GetVehicle()->GetPosition().y);
  carSprite_.setRotation(angleDeg); 
}

void RaceScene::render() {
  manager_.getWindow().draw(backgroundSprite_);
  manager_.getWindow().draw(carSprite_);


  // Vehicle *car = car_->GetVehicle();
  // b2ShapeId shapeId = car->GetShape();
  // if (b2Shape_IsValid(shapeId)) {
  //   b2Polygon poly = b2Shape_GetPolygon(shapeId);
  //   // sf::ConvexShape car_body_;
  //   car_body_.setPointCount(poly.count);
  //   for (int i = 0; i < poly.count; ++i)
  //     car_body_.setPoint(
  //         i, sf::Vector2f(poly.vertices[i].x * 20, poly.vertices[i].y * 20));
  //   car_body_.setFillColor(sf::Color::Red);
  //   manager_.getWindow().draw(car_body_);
  // }

  // Draw tires
  // std::vector<TDTire *> tires = car_->GetVehicle()->GetTires();
  // for (TDTire *tire : tires) {
  //   b2BodyId tireBody = tire->GetBody();
  //   b2ShapeId tireShape = tire->GetShape();
  //   if (b2Shape_IsValid(tireShape)) {
  //     b2Polygon poly = b2Shape_GetPolygon(tireShape);
  //     sf::ConvexShape car_body_;
  //     car_body_.setPointCount(poly.count);
  //     for (int i = 0; i < poly.count; ++i)
  //       car_body_.setPoint(
  //           i, sf::Vector2f(poly.vertices[i].x * 20, poly.vertices[i].y *
  //           20));
  //     car_body_.setFillColor(sf::Color::White);
  //     manager_.getWindow().draw(car_body_);
  //   }
  // }
}
