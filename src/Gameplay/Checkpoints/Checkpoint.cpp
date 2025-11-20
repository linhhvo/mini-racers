#include "Checkpoint.hpp"
#include <iostream>
#include <string>

Checkpoint::Checkpoint(b2WorldId world, b2Vec2 position, float width,
                       float height)
    : position_(position) {
  // box2d static body
  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_staticBody;
  bodyDef.position = position_;
  body_ = b2CreateBody(world, &bodyDef);

  // box2d sensor shape
  b2Polygon box = b2MakeBox(width, height);
  b2ShapeDef shapeDef = b2DefaultShapeDef();
  shapeDef.enableContactEvents = true;
  shapeDef.enableHitEvents = false;
  shapeDef.enableSensorEvents = true;
  shapeDef.isSensor = true;
  shapeDef.userData = (void *)"checkpoint";

  // car creator
  checkpoint_ = b2CreatePolygonShape(body_, &shapeDef, &box);
}

b2BodyId Checkpoint::GetBody() { return body_; }

b2ShapeId Checkpoint::GetShapeId() const { return checkpoint_; }

const b2Vec2 &Checkpoint::GetPosition() const { return position_; }

bool Checkpoint::isActivated() const { return activated_; }

// activate tmx animation
void Checkpoint::activate() {
  activated_ = true;
  timer_ = 0.5f;
};

void Checkpoint::reset() {
  activated_ = false;
  timer_ = 0.f;
}

// update tmx animation
void Checkpoint::update(float dt) {
  if (activated_) {
    timer_ -= dt;
    if (timer_ <= 0.0f) {
      reset();
    }
  }
};
