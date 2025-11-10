#include "Vehicle.hpp"
#include "System/InputManager.hpp"
#include <box2d/box2d.h>
#include <iostream>
#include <vector>

Vehicle::Vehicle(b2WorldId world, b2Vec2 pos, float height, float width)
    : world_(world) {
  // create chassis body
  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_dynamicBody;
  bodyDef.angularDamping = 1.0f;
  bodyDef.linearDamping = 0.3f;
  bodyDef.position = pos;
  body = b2CreateBody(world, &bodyDef);
  b2Body_SetAngularDamping(body,
                           0.6f); // Reduced from 4.0f for faster turning
  b2Body_SetLinearDamping(body, 3.0f);

  // Box Polygon shape
  b2Polygon box = b2MakeBox(height, width);
  // Tire material (can be changed for future vehicle types)
  b2SurfaceMaterial material = b2SurfaceMaterial();
  material.friction = 0.8f;
  material.restitution = 0.0f;
  material.rollingResistance = 0.0f;
  // Shape def
  b2ShapeDef shapeDef = b2DefaultShapeDef();
  shapeDef.density = 1;
  // void *data = static_cast<void *>(&idx);
  // shapeDef.userData = data;
  shapeDef.userData = this;
  shapeDef.enableContactEvents = true;
  shapeDef.enableHitEvents = true;
  shapeDef.enablePreSolveEvents =
      false; // true if we want to disable collision (powerup maybe?)
  shapeDef.enableSensorEvents = true;
  shapeDef.isSensor = false;
  shapeDef.material = material;

  // car creator
  car = b2CreatePolygonShape(body, &shapeDef, &box);
};

void Vehicle::updateKey(InputManager::InputAction action, bool pressed) {
  switch (action) {
  case InputManager::UP:
    up = pressed;
    break;
  case InputManager::LEFT:
    left = pressed;
    break;
  case InputManager::DOWN:
    down = pressed;
    break;
  case InputManager::RIGHT:
    right = pressed;
    break;
  default:
    break;
  }
}

void Vehicle::update(float dt) {
  handleDrift();
  if (forceStrength < targetForceStrength) {
    forceStrength =
        std::min(forceStrength + accelerationRate * dt, targetForceStrength);
  } else if (forceStrength > targetForceStrength) {
    forceStrength =
        std::max(forceStrength - brakeRate * dt, targetForceStrength);
  }

  if (left) {
    b2Body_SetAngularVelocity(body, -3.0f);
  } else if (right) {
    b2Body_SetAngularVelocity(body, 3.0f);
  } else {
    b2Body_SetAngularVelocity(body, 0.0f);
  }
  b2Vec2 localHood = {0.0f, 2.0f};

  if (up) {
    b2Vec2 forward = b2Body_GetWorldVector(body, b2Vec2{0.0f, -1.0f});
    b2Vec2 force = {forward.x * forceStrength, forward.y * forceStrength};
    b2Vec2 worldHood = b2Body_GetWorldPoint(body, localHood);
    b2Body_ApplyForce(body, force, worldHood, true);
  }
  if (down) {
    b2Vec2 backward = b2Body_GetWorldVector(body, b2Vec2{0.0f, 1.0f});
    b2Vec2 force = {backward.x * forceStrength, backward.y * forceStrength};
    b2Vec2 worldHood = b2Body_GetWorldPoint(body, localHood);
    b2Body_ApplyForce(body, force, worldHood, true);
  }
}
