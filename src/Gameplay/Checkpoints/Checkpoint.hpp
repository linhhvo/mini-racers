#pragma once

#include "box2d/math_functions.h"
#include <box2d/box2d.h>
#include <vector>

class Checkpoint {
public:
  Checkpoint(b2WorldId world, b2Vec2 position, float width, float height);
  ~Checkpoint() = default;

  b2BodyId GetBody();
  b2ShapeId GetShapeId() const;
  const b2Vec2 &GetPosition() const;
  uint32_t getCurrentTileID() const;

  void activate();
  void update(float dt);
  void setAnimationFrames(const std::vector<uint32_t> &frames,
                          const std::vector<float> &durations);

  bool isActivated() const;

private:
  b2BodyId body_;
  b2ShapeId checkpoint_;
  b2Vec2 position_;

  bool activated_ = false;

  std::vector<uint32_t> frames_;      // GIDs from TMX tileset
  std::vector<float> frameDurations_; // seconds per frame
  float timer_ = 0.f;
  int currentFrame_ = 0;

  uint32_t currentTileID_ = 0; // used for rendering
};
