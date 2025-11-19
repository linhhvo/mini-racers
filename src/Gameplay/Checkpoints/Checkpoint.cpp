#include "Checkpoint.hpp"

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

uint32_t Checkpoint::getCurrentTileID() const { return currentTileID_; }

bool Checkpoint::isActivated() const { return activated_; }

// activate tmx animation
void Checkpoint::activate() {
  activated_ = true;
  currentFrame_ = 0;
  timer_ = 0;

  if (!frames_.empty()) {
    currentTileID_ = frames_[0];
  }
};

// update tmx animation
void Checkpoint::update(float dt) {
  if (!activated_ || frames_.empty())
    return;

  timer_ += dt;

  if (timer_ >= frameDurations_[currentFrame_]) {
    timer_ -= frameDurations_[currentFrame_];
    currentFrame_++;

    // Animation finished → deactivate
    if (currentFrame_ >= (int)frames_.size()) {
      activated_ = false;
      currentFrame_ = 0;
      return;
    }

    currentTileID_ = frames_[currentFrame_];
  }
};

void Checkpoint::setAnimationFrames(const std::vector<uint32_t> &frames,
                                    const std::vector<float> &durations) {
  frames_ = frames;
  frameDurations_ = durations;

  currentFrame_ = 0;
  timer_ = 0;

  if (!frames_.empty())
    currentTileID_ = frames_[0];
}
