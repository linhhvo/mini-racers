#pragma once
#include "Obstacles/Obstacle.hpp"
#include "System/InputManager.hpp"
#include "Vehicle.hpp"
#include <box2d/box2d.h>

// Testing world for MicroMachine
class GameWorld {
public:
  GameWorld(b2WorldId world) : world_(world) {
    b2World_SetGravity(world, b2Vec2{0.0f, 0.0f});
  }

  ~GameWorld() {
    for (int i = 0; i < nofCars_; i++) {
      delete cars[i];
    }
    cars.clear();
  }

  void VehicleFactory(int nofCars, std::vector<b2Vec2> startPos,
                      std::vector<std::pair<float, float>> sizes,
                      std::vector<VehicleSpec> specs) {
    nofCars_ = nofCars;
    for (int i = 0; i < nofCars; i++) {
      auto car = new Vehicle(world_, startPos[i], sizes[i].first,
                             sizes[i].second, specs[i]);
      cars.push_back(car);
    }
  }

  void ObstacleFactory(int nofObstacles, std::vector<b2Vec2> startPos,
                       std::vector<std::pair<float, float>> sizes) {
    nofObstacles_ = nofObstacles;
    for (int i = 0; i < nofObstacles; i++) {
      auto obstacle =
          new Obstacle(world_, startPos[i], sizes[i].first, sizes[i].second);
      obstacles.push_back(obstacle);
    }
  }

  Vehicle *GetVehicle(int idx) { return cars[idx]; }

  Obstacle *GetObstacle(int idx) { return obstacles[idx]; }

  void Keyboard(InputManager::InputAction action, int idx, bool pressed) {
    cars[idx]->updateKey(action, pressed);
  }

  static GameWorld *Create(b2WorldId world) { return new GameWorld(world); }

private:
  int nofCars_;
  int nofObstacles_;
  std::vector<Vehicle *> cars;
  std::vector<Obstacle *> obstacles;
  int m_controlState;
  b2WorldId world_;
};
