#pragma once

#include "box2d/math_functions.h"
#include <box2d/box2d.h>

// class SoftObstacle {
// public:
//   SoftObstacle(b2WorldId world, b2Vec2 position, float height, float width,
//                float strength)
//       : postion_(position), strength_(strength) {
//     b2BodyDef bodyDef = b2DefaultBodyDef();
//     bodyDef.type = b2_staticBody;
//     bodyDef.position = postion_;
//     body = b2CreateBody(world, &bodyDef);
//
//     // Box Polygon shape
//     b2Polygon box = b2MakeBox(height, width);
//     // Tire material (can be changed for future vehicle types)
//     b2SurfaceMaterial material = b2SurfaceMaterial();
//     material.friction = 0.8f;
//     material.restitution = 0.0f;
//     material.rollingResistance = 0.0f;
//     // Shape def
//     b2ShapeDef shapeDef = b2DefaultShapeDef();
//     shapeDef.enableContactEvents = false;
//     shapeDef.enableHitEvents = false;
//     shapeDef.enableSensorEvents = true;
//     shapeDef.isSensor = true;
//     shapeDef.userData = (void *)"softObstacle";
//     // car creator
//     softObstacle = b2CreatePolygonShape(body, &shapeDef, &box);
//   }
//   ~SoftObstacle() = default;
//
//   b2BodyId GetBody() { return body; }
//   b2ShapeId GetShapeId() { return softObstacle; }
//   float GetStrength() { return strength_; }
//
// private:
//   float strength_;
//   b2BodyId body;
//   b2ShapeId softObstacle;
//   b2Vec2 postion_;
// };
