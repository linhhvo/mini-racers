#include "RaceScene.hpp"
#include "Scenes/MenuScene.hpp"
#include "Scenes/SceneManager.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <iostream>

const std::string IMAGE_FILE{"./assets/backgrounds/RaceScene.jpg"};

RaceScene::RaceScene(SceneManager &manager) : manager_(manager) {
  sf::RenderWindow &window = manager_.getWindow();

  if (!backgroundTexture_.loadFromFile(IMAGE_FILE)) {
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

  //box2D 
  b2WorldDef worldDef = b2DefaultWorldDef();
  worldDef.gravity = {0.0f, 0.0f};
  world_ = b2CreateWorld(&worldDef);

  car_ = iforce2d_TopdownCar::Create(world_);
}

void RaceScene::handleEvent(const sf::Event &event) {
  if (event.type == sf::Event::KeyPressed) {
    switch(event.key.code) {
      case sf::Keyboard::W: car_->Keyboard('w'); break;
      case sf::Keyboard::A: car_->Keyboard('a'); break;
      case sf::Keyboard::S: car_->Keyboard('s'); break;
      case sf::Keyboard::D: car_->Keyboard('d'); break;
      case sf::Keyboard::Escape: manager_.switchScene<MenuScene>(manager_); break;
    }
  }
  else if (event.type == sf::Event::KeyReleased) {
    switch(event.key.code) {
      case sf::Keyboard::W: car_->KeyboardUp('w'); break;
      case sf::Keyboard::A: car_->KeyboardUp('a'); break;
      case sf::Keyboard::S: car_->KeyboardUp('s'); break;
      case sf::Keyboard::D: car_->KeyboardUp('d'); break;
    }
  }
}

void RaceScene::update(float dt) {
    b2World_Step(world_, dt, 4);
}

void RaceScene::render() { 
  manager_.getWindow().draw(backgroundSprite_); 
  Vehicle* car = car_->GetVehicle();
    b2ShapeId shapeId = car->GetShape();
    if (b2Shape_IsValid(shapeId))
    {
        b2Polygon poly = b2Shape_GetPolygon(shapeId);
        sf::ConvexShape sfShape;
        sfShape.setPointCount(poly.count);
        for (int i = 0; i < poly.count; ++i)
            sfShape.setPoint(i, sf::Vector2f(poly.vertices[i].x * 20, poly.vertices[i].y * 20));
        sfShape.setFillColor(sf::Color::Red);
        manager_.getWindow().draw(sfShape);
    }

    // Draw tires
    std::vector<TDTire*> tires = car_->GetVehicle()->GetTires();
    for (TDTire* tire : tires)
    {
        b2BodyId tireBody = tire->GetBody();
        b2ShapeId tireShape = tire->GetShape();
        if (b2Shape_IsValid(tireShape))
        {
            b2Polygon poly = b2Shape_GetPolygon(tireShape);
            sf::ConvexShape sfShape;
            sfShape.setPointCount(poly.count);
            for (int i = 0; i < poly.count; ++i)
                sfShape.setPoint(i, sf::Vector2f(poly.vertices[i].x * 20, poly.vertices[i].y * 20));
            sfShape.setFillColor(sf::Color::White);
            manager_.getWindow().draw(sfShape);
        }
    }
}
