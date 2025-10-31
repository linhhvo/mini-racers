#pragma once
#include "Scenes/Scene.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <memory>
#include <utility>

class SceneManager {
public:
  SceneManager(sf::RenderWindow &window) : window_(window) {};

  template <typename T, typename... Args> void switchScene(Args &&...args) {
    currentScene = std::make_unique<T>(std::forward<Args>(args)...);
  }

  Scene *getCurrentScene() { return currentScene.get(); }

  sf::RenderWindow &getWindow() { return window_; }

private:
  std::unique_ptr<Scene> currentScene;
  sf::RenderWindow &window_;
};
