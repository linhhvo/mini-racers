#pragma once
#include "Vehicle.hpp"
#include <box2d/box2d.h>

class iforce2d_TopdownCar {
public:
  iforce2d_TopdownCar(b2WorldId world, int nofCars, float startX, float startY, float spacing) : world_(world), nofCars_(nofCars) {
    b2World_SetGravity(world, b2Vec2{0.0f, 0.0f});

    // set up ground areas
    {
      b2Polygon ground = b2MakeBox(9, 7);

      b2SurfaceMaterial material = b2SurfaceMaterial();
      material.friction = 0.8f;
      material.restitution = 0.1f;
      material.rollingResistance = 0.1f;

      // Shape def
      b2ShapeDef shapeDef = b2DefaultShapeDef();
      shapeDef.density = 1;
      shapeDef.enableContactEvents = true;
      shapeDef.enableHitEvents = true;
      shapeDef.enableSensorEvents = true;
      shapeDef.isSensor = true;
      shapeDef.material = material;
    }

    for (int i =0; i < nofCars_; i++) {
      auto car = new Vehicle(world_);
      b2Rot rot = b2MakeRot(0.0f);
      b2Vec2 startPos {startX + i * spacing, startY};
      b2Body_SetTransform(car->GetBody(), startPos, rot);
      cars.push_back(car);
    }

    m_controlState = 0;
  }

  ~iforce2d_TopdownCar() {
    for (int i = 0; i < nofCars_; i++) {
      delete cars[i];
    }
    cars.clear();
  }
  Vehicle *GetVehicle(int idx) { return cars[idx]; }

  void Keyboard(unsigned char key, bool pressed, int idx) {
    if (idx >= 0 && idx < nofCars_) {
      cars[idx]->updateKey(key, pressed);
    }
  }

  static iforce2d_TopdownCar *Create(b2WorldId world, int nofCars, float startingX, float startingY, float spacing) {
    return new iforce2d_TopdownCar(world, nofCars, startingX, startingY, spacing);
  }

private:
  int nofCars_;
  std::vector<Vehicle*> cars;
  int m_controlState;
  b2WorldId world_;
};
