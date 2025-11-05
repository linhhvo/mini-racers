#pragma once
#include "Checkpoint.hpp"
#include "Obstacle.hpp"
#include "SoftObstacle.hpp"
#include "System/InputManager.hpp"
#include "Vehicle.hpp"
#include <box2d/box2d.h>
#include <iostream>

class iforce2d_TopdownCar {
public:
  iforce2d_TopdownCar(b2WorldId world, int nofCars, float startX, float startY,
                      float spacing)
      : world_(world), nofCars_(nofCars) {
    b2World_SetGravity(world, b2Vec2{0.0f, 0.0f});
<<<<<<< HEAD
    for (int i = 0; i < nofCars_; i++) {
      b2Vec2 startPos{startX + i * spacing, startY};
=======
>>>>>>> ceb83a1 (refactor player input)
      std::string idx = std::to_string(i);
      auto car = new Vehicle(world_, startPos, idx);
      cars.push_back(car);
    }

    b2Vec2 obPos{startX + 5, startY + 5};
    auto obstacle = new Obstacle(world_, obPos);
    obstacles.push_back(obstacle);

    b2Vec2 checkpointPos{startX + 5, startY - 4};
    checkpoint = new Checkpoint(world_, checkpointPos);

    b2Vec2 sobPos{startX - 2, startY - 3};
    auto softObstacle = new SoftObstacle(world_, sobPos);
    softObstacles.push_back(softObstacle);
  }

  ~iforce2d_TopdownCar() {
    for (int i = 0; i < nofCars_; i++) {
      delete cars[i];
    }
    cars.clear();
  }
  Vehicle *GetVehicle(int idx) { return cars[idx]; }

  Obstacle *GetObstacle(int idx) { return obstacles[idx]; }

  Checkpoint *GetCheckPoint() { return checkpoint; }

  SoftObstacle *GetSoftObstacle(int idx) { return softObstacles[idx]; }

  void Keyboard(InputManager::InputAction action, int idx, bool pressed) {
    cars[idx]->updateKey(action, pressed);
  }

  static iforce2d_TopdownCar *Create(b2WorldId world, int nofCars,
                                     float startingX, float startingY,
                                     float spacing) {
    return new iforce2d_TopdownCar(world, nofCars, startingX, startingY,
                                   spacing);
  }

private:
  int nofCars_;
  std::vector<Vehicle *> cars;
  std::vector<Obstacle *> obstacles;
  std::vector<SoftObstacle *> softObstacles;
  Checkpoint *checkpoint;
  int m_controlState;
  b2WorldId world_;
};
