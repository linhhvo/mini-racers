#pragma once

#include "System/Config.hpp"
#include "System/InputManager.hpp"

struct GameContext {
  Config *config;
  InputManager *input;
};
