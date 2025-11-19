#pragma once

#include "box2d/math_functions.h"
#include <box2d/box2d.h>

class Obstacle {
public:
  Obstacle(b2WorldId world, b2Vec2 position, float height, float width)
      : postion_(position) {
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_staticBody;
    bodyDef.position = postion_;
    body = b2CreateBody(world, &bodyDef);

    // Box Polygon shape
    b2Polygon box = b2MakeBox(height, width);
    // Tire material (can be changed for future vehicle types)
    b2SurfaceMaterial material = b2SurfaceMaterial();
    material.friction = 0.8f;
    material.restitution = 0.0f;
    material.rollingResistance = 0.0f;
    // Shape def
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.enableContactEvents = true;
    shapeDef.enableHitEvents = true;
    shapeDef.enablePreSolveEvents =
        false; // true if we want to disable collision (powerup maybe?)

    // car creator
    obstacle = b2CreatePolygonShape(body, &shapeDef, &box);
  }
  ~Obstacle() = default;

  b2BodyId GetBody() { return body; }

private:
  b2BodyId body;
  b2ShapeId obstacle;
  b2Vec2 postion_;
};
