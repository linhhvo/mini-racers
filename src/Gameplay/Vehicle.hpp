#pragma once

#include "../System/InputManager.hpp"
#include "VehicleSpec.hpp"
#include "box2d/math_functions.h"
#include <box2d/box2d.h>

class Vehicle {
public:
  Vehicle(b2WorldId world, b2Vec2 pos, float width, float height,
          VehicleSpec specs);
  ~Vehicle() = default;

  // movement functions
  void updateKey(InputManager::InputAction action, bool pressed);
  void update(float dt);

  void SetTargetForceStrength(float strength) {
    targetForceStrength = strength;
  }
  void SetAccelerationRate(float rate) { accelerationRate = rate; }

  void Reset() {
    this->SetTargetForceStrength(baseTargetForceStrength);
    this->SetAccelerationRate(baseAccelerationRate);
  }

  void HitSoftObstacle(float desiredStrength, float accelerationStrength) {
    this->SetTargetForceStrength(targetForceStrength * desiredStrength);
    this->SetAccelerationRate(accelerationRate * accelerationStrength);
  }

  // getter functions
  b2BodyId GetBody() { return body; }
  b2ShapeId GetShape() { return car; };
  b2Vec2 GetPosition() { return b2Body_GetPosition(body); };
  float GetSpeed() { return forceStrength; }

private:
  b2Vec2 getLateralVelocity() const {
    b2Vec2 currentRightNormal = b2Body_GetWorldVector(body, b2Vec2{1.0f, 0.0f});
    b2Vec2 linearVel = b2Body_GetLinearVelocity(body);
    return b2Dot(currentRightNormal, linearVel) * currentRightNormal;
  }

  b2Vec2 getForwardVelocity() const {
    b2Vec2 currentForwardNormal =
        b2Body_GetWorldVector(body, b2Vec2{0.0f, 1.0f});
    b2Vec2 linearVel = b2Body_GetLinearVelocity(body);
    return b2Dot(currentForwardNormal, linearVel) * currentForwardNormal;
  }

  void handleDrift() {
    float drift = 0.0f;
    b2Vec2 forwardSpeed = getForwardVelocity();
    b2Vec2 lateralSpeed = getLateralVelocity();
    b2Vec2 desiredSpeed = {forwardSpeed.x + lateralSpeed.x * drift,
                           forwardSpeed.y + lateralSpeed.y * drift};
    b2Body_SetLinearVelocity(body, desiredSpeed);
  }

  b2BodyId body;
  b2WorldId world_;
  b2ShapeId car;
  bool up = false;
  bool left = false;
  bool down = false;
  bool right = false;
  float forceStrength = 0.0f;
  float brakeRate = 5.0f;
  float baseTargetForceStrength;
  float baseAccelerationRate;
  float targetForceStrength = baseTargetForceStrength;
  float accelerationRate = baseAccelerationRate;
};
