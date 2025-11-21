#pragma once
#include "Obstacles/Obstacle.hpp"
#include "System/InputManager.hpp"
#include "Vehicle.hpp"
#include <box2d/box2d.h>
#include <memory>

// Testing world for MicroMachine
class GameWorld {
public:
  GameWorld(b2WorldId world) : world_(world) {
    b2World_SetGravity(world, b2Vec2{0.0f, 0.0f});
  }

  ~GameWorld() = default;

  void VehicleFactory(int nofCars, std::vector<b2Vec2> startPos,
                      std::vector<std::pair<float, float>> sizes,
                      std::vector<VehicleSpec> specs) {
    nofCars_ = nofCars;
    for (int i = 0; i < nofCars; i++) {
      cars.push_back(std::make_unique<Vehicle>(
          world_, startPos[i], sizes[i].first, sizes[i].second, specs[i]));
    }
  }

  void ObstacleFactory(int nofObstacles, std::vector<b2Vec2> startPos,
                       std::vector<std::pair<float, float>> sizes) {
    nofObstacles_ = nofObstacles;
    for (int i = 0; i < nofObstacles; i++) {
      obstacles.push_back(std::make_unique<Obstacle>(
          world_, startPos[i], sizes[i].first, sizes[i].second));
    }
  }

  Vehicle *GetVehicle(int idx) { return cars[idx].get(); }

  Obstacle *GetObstacle(int idx) { return obstacles[idx].get(); }

  void Keyboard(InputManager::InputAction action, int idx, bool pressed) {
    cars[idx]->updateKey(action, pressed);
  }

  static std::unique_ptr<GameWorld> Create(b2WorldId world) {
    return std::make_unique<GameWorld>(world);
  }

private:
  int nofCars_ = 0;
  int nofObstacles_ = 0;
  std::vector<std::unique_ptr<Vehicle>> cars;
  std::vector<std::unique_ptr<Obstacle>> obstacles;
  int m_controlState;
  b2WorldId world_;
};
