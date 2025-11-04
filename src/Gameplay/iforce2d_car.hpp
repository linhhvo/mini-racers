#pragma once
#include "Vehicle.hpp"
#include "Obstacle.hpp"
#include <box2d/box2d.h>

class iforce2d_TopdownCar {
public:
 iforce2d_TopdownCar(b2WorldId world, int nofCars, float startX, float startY, float spacing) : world_(world), nofCars_(nofCars) {
    b2World_SetGravity(world, b2Vec2{0.0f, 0.0f});

    // set up ground areas
    {
      b2BodyDef bodyDef = b2DefaultBodyDef();
      bodyDef.type = b2_staticBody;
      bodyDef.position = b2Vec2{0.0f, 0.0f};
      b2BodyId body = b2CreateBody(world, &bodyDef);
      
      b2Polygon groundShape = b2MakeBox(9.0f, 7.0f);

      b2SurfaceMaterial material = b2SurfaceMaterial();
      material.friction = 0.8f;
      material.restitution = 0.1f;
      material.rollingResistance = 0.1f;

      // Shape def
      b2ShapeDef shapeDef = b2DefaultShapeDef();
      shapeDef.density = 1;
      shapeDef.enableContactEvents = true;
      shapeDef.enableHitEvents = true;
      shapeDef.enableSensorEvents = false;
      shapeDef.material = material;
      b2CreatePolygonShape(body, &shapeDef, &groundShape);
    }

    for (int i =0; i < nofCars_; i++) {
      b2Vec2 startPos {startX + i * spacing, startY};
      auto car = new Vehicle(world_, startPos);
      cars.push_back(car);
    }
    b2Vec2 obPos {startX + 5, startY + 5};
    auto obstacle = new Obstacle(world_, obPos);
    obstacles.push_back(obstacle);
  }

  ~iforce2d_TopdownCar() {
    for (int i = 0; i < nofCars_; i++) {
      delete cars[i];
    }
    cars.clear();
  }
  Vehicle *GetVehicle(int idx) { return cars[idx]; }

  Obstacle *GetObstacle(int idx) {return obstacles[idx];}

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
  std::vector<Obstacle*> obstacles;
  int m_controlState;
  b2WorldId world_;
};

