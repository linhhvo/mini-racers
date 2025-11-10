#pragma once
#include "Boosters.hpp"
#include "Obstacle.hpp"
#include "SoftObstacle.hpp"
#include "System/InputManager.hpp"
#include "Vehicle.hpp"
#include <box2d/box2d.h>
#include <iostream>

// Testing world for MicroMachine
class iforce2d_TopdownCar {
public:
  // iforce2d_TopdownCar(b2WorldId world, int nofCars, int nofObstacles,
  //                     int nofSoft, int nofBooster, float startX, float
  //                     startY, float spacing)
  //     : world_(world), nofCars_(nofCars), nofObstacles_(nofObstacles),
  //       nofSoft_(nofSoft), nofBoosters_(nofBooster) {
  //   b2World_SetGravity(world, b2Vec2{0.0f, 0.0f});
  //   // cars
  //   for (int i = 0; i < nofCars_; i++) {
  //     b2Vec2 startPos{startX + i * spacing, startY};

  iforce2d_TopdownCar(b2WorldId world) : world_(world) {
    b2World_SetGravity(world, b2Vec2{0.0f, 0.0f});
  }

  void VehicleFactory(int nofCars, std::vector<b2Vec2> startPos,
                      std::vector<std::pair<float, float>> sizes) {
    nofCars_ = nofCars;
    for (int i = 0; i < nofCars; i++) {
      auto car =
          new Vehicle(world_, startPos[i], sizes[i].first, sizes[i].second);
      cars.push_back(car);
    }
  };

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

  Booster *GetBooster(int idx) { return boosters[idx]; }

  void Keyboard(InputManager::InputAction action, int idx, bool pressed) {
    cars[idx]->updateKey(action, pressed);
  }

  // static iforce2d_TopdownCar *Create(b2WorldId world, int nofCars,
  //                                    int nofObstacles, int nofSoft,
  //                                    int nofBooster, float startX, float
  //                                    startY, float spacing) {
  //   return new iforce2d_TopdownCar(world, nofCars, nofObstacles, nofSoft,
  //                                  nofBooster, startX, startY, spacing);
  static iforce2d_TopdownCar *Create(b2WorldId world) {
    return new iforce2d_TopdownCar(world);
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
