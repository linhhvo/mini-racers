#pragma once
#include <box2d/box2d.h>

struct VehicleSpec {
  float angularDamping = 0.6f;
  float linearDamping = 3.0f;
  float friction = 0.8f;
  float restitution = 0.0f;
  float rollingResistance = 0.0f;
  float brakeRate = 5.0f;
  float baseTargetForceStrength = 3.0f;
  float baseAccelerationRate = 1.0f;
};
