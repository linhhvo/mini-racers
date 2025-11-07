#pragma once
#include "Obstacle.hpp"
#include "SoftObstacle.hpp"
#include "Boosters.hpp"
#include "System/InputManager.hpp"
#include "Vehicle.hpp"
#include <box2d/box2d.h>
#include <iostream>

// Testing world for MicroMachine
class iforce2d_TopdownCar {
public:
  iforce2d_TopdownCar(b2WorldId world, int nofCars, int nofObstacles, int nofSoft, int nofBooster, float startX, float startY,
                      float spacing)
      : world_(world), nofCars_(nofCars), nofObstacles_(nofObstacles), nofSoft_(nofSoft), nofBoosters_(nofBooster)  {
    b2World_SetGravity(world, b2Vec2{0.0f, 0.0f});
    // cars
    for (int i = 0; i < nofCars_; i++) {
      b2Vec2 startPos{startX + i * spacing, startY};
      std::string idx = std::to_string(i);
      auto car = new Vehicle(world_, startPos, idx);
      cars.push_back(car);
    }

    // obstacles
    for (int i = 0; i < nofObstacles_; i++) {
      b2Vec2 obPos{startX + (i+1) * spacing * 2.0f, startY + (i%2 ? spacing : -spacing)};
      auto obstacle = new Obstacle(world_, obPos);
      obstacles.push_back(obstacle);
    }

    // soft obstacle
    for (int i = 0; i < nofSoft_; i++) {
      b2Vec2 sofPos{startX + (i+1) * spacing * 1.5f, startY + (i%3 -1) * spacing};
      auto softObstacle = new SoftObstacle(world_, sofPos, 5.0f);
      softObstacles.push_back(softObstacle);
    }

    // Booster
    for (int i = 0; i < nofBoosters_; i++) {
      b2Vec2 bosPos{startX - (i+1) * spacing * 1.5f, startY + (i%3 -1) * spacing};
      auto booster = new Booster(world_, bosPos, 50.0f);
      boosters.push_back(booster);
    }
    std::cout << "iforce2d: cars=" << cars.size()
          << " obstacles=" << obstacles.size()
          << " soft=" << softObstacles.size()
          << " boosters=" << boosters.size() << "\n";
  }


  ~iforce2d_TopdownCar() {
    for (int i = 0; i < nofCars_; i++) {
      delete cars[i];
    }
    cars.clear();
  }
  Vehicle *GetVehicle(int idx) { return cars[idx]; }

  Obstacle *GetObstacle(int idx) { return obstacles[idx]; }

  // Checkpoint *GetCheckPoint() { return checkpoint; }

  SoftObstacle *GetSoftObstacle(int idx) { return softObstacles[idx]; }

  Booster *GetBooster(int idx) {return boosters[idx];}

  void Keyboard(InputManager::InputAction action, int idx, bool pressed) {
    cars[idx]->updateKey(action, pressed);
  }

  static iforce2d_TopdownCar *Create(b2WorldId world, int nofCars, int nofObstacles, int nofSoft, int nofBooster, float startX, float startY, float spacing) {
    return new iforce2d_TopdownCar(world, nofCars, nofObstacles, nofSoft, nofBooster, startX, startY, spacing);
  }

private:
  int nofCars_;
  int nofSoft_;
  int nofObstacles_;
  int nofBoosters_;
  // Checkpoint *checkpoint;
  int m_controlState;
  b2WorldId world_;

  std::vector<Vehicle *> cars;
  std::vector<Obstacle *> obstacles;
  std::vector<SoftObstacle *> softObstacles;
  std::vector<Booster *> boosters;
};
