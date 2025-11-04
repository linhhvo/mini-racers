#pragma once

#include "../System/InputManager.hpp"
#include "box2d/math_functions.h"
#include <box2d/box2d.h>
#include <vector>
#include <iostream>


class Obstacle {
public:
  Obstacle(b2WorldId world, b2Vec2 position): postion_(position) {
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_staticBody;
    bodyDef.position = postion_;
    body = b2CreateBody(world, &bodyDef);

    // Box Polygon shape
    b2Polygon box = b2MakeBox(0.5f, 0.5f);
    // Tire material (can be changed for future vehicle types)
    b2SurfaceMaterial material = b2SurfaceMaterial();
    material.friction = 0.8f;
    material.restitution = 0.0f;
    material.rollingResistance = 0.0f;
    // Shape def
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.enableContactEvents = true;
    shapeDef.enableHitEvents = true;
    shapeDef.enablePreSolveEvents = false; // true if we want to disable collision (powerup maybe?)

    // car creator
    obstacle = b2CreatePolygonShape(body, &shapeDef, &box);
  }
  ~Obstacle() = default;

private:
  b2BodyId body;
  b2ShapeId obstacle;
  b2Vec2 postion_;
};