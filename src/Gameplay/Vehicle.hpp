#pragma once

#include "../System/InputManager.hpp"
#include "box2d/math_functions.h"
#include <box2d/box2d.h>
#include <vector>
#include <iostream>

#ifndef DEGTORAD
#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f
#endif

class Vehicle {
public:
  Vehicle(b2WorldId world, b2Vec2 pos);
  ~Vehicle() = default;

  //movement functions
  void updateKey(char key, bool pressed);
  void update();

  //getter functions
  b2BodyId GetBody() {return body;}
  b2ShapeId GetShape() { return car; };
  b2Vec2 GetPosition() { return b2Body_GetPosition(body); };

private:
  b2Vec2 getLateralVelocity() const {
    b2Vec2 currentRightNormal = b2Body_GetWorldVector(body, b2Vec2{1.0f, 0.0f});
    b2Vec2 linearVel = b2Body_GetLinearVelocity(body);
    return b2Dot(currentRightNormal, linearVel) * currentRightNormal;
  }

  b2Vec2 getForwardVelocity() const {
    b2Vec2 currentForwardNormal = b2Body_GetWorldVector(body, b2Vec2{0.0f, 1.0f});
    b2Vec2 linearVel = b2Body_GetLinearVelocity(body);
    return b2Dot(currentForwardNormal, linearVel) * currentForwardNormal;
  }

  void handleDrift() {
    float drift = 0.0f;
    b2Vec2 forwardSpeed = getForwardVelocity();
    b2Vec2 lateralSpeed = getLateralVelocity();
    b2Vec2 desiredSpeed = {forwardSpeed.x + lateralSpeed.x * drift, forwardSpeed.y + lateralSpeed.y * drift};
    b2Body_SetLinearVelocity(body, desiredSpeed);
  }

  b2BodyId body;
  b2WorldId world_;
  b2ShapeId car;
  bool up = false;
  bool left = false;
  bool down = false;
  bool right = false;
};
