#include "Core/App.hpp"
#include "System/Config.hpp"
#include "System/GameContext.hpp"
#include "System/InputManager.hpp"

int main() {
  Config config;
  InputManager input;

  GameContext context{&config, &input};

  App app(context);
  app.run();
  return 0;
}
