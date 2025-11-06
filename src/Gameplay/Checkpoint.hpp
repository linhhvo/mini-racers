#pragma once

#include "../System/InputManager.hpp"
#include "box2d/math_functions.h"
#include <box2d/box2d.h>
#include <vector>
#include <iostream>


class Checkpoint {
public:
  Checkpoint(b2WorldId world, b2Vec2 position): postion_(position) {
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_staticBody;
    bodyDef.position = postion_;
    body = b2CreateBody(world, &bodyDef);

    // Box Polygon shape
    b2Polygon box = b2MakeBox(0.5f, 2.0f);
    // Tire material (can be changed for future vehicle types)
    b2SurfaceMaterial material = b2SurfaceMaterial();
    material.friction = 0.8f;
    material.restitution = 0.0f;
    material.rollingResistance = 0.0f;
    // Shape def
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.enableContactEvents = true;
    shapeDef.enableHitEvents = false;
    shapeDef.enableSensorEvents = true;
    shapeDef.isSensor = true;
    shapeDef.userData = (void*)"checkpoint";
    // car creator
    checkpoint = b2CreatePolygonShape(body, &shapeDef, &box);
  }
  ~Checkpoint() = default;

  b2BodyId GetBody() {return body;}
  b2ShapeId GetShapeId() const { return checkpoint; }

private:
  b2BodyId body;
  b2ShapeId checkpoint;
  b2Vec2 postion_;
};