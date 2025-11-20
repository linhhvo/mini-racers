#pragma once

#include "box2d/math_functions.h"
#include <box2d/box2d.h>
#include <string>

class Checkpoint {
public:
  Checkpoint(b2WorldId world, b2Vec2 position, float width, float height);
  ~Checkpoint() = default;

  b2BodyId GetBody();
  b2ShapeId GetShapeId() const;
  const b2Vec2 &GetPosition() const;

  void activate();
  void reset();
  void update(float dt);
  bool isActivated() const;

private:
  b2BodyId body_;
  b2ShapeId checkpoint_;
  b2Vec2 position_;

  bool activated_ = false;
  float timer_ = 0.f;
};
