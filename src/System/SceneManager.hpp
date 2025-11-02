#pragma once
#include "Scenes/Scene.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <memory>
#include <vector>

class SceneManager {
public:
  SceneManager(sf::RenderWindow &window);

  // switch to a different scene completely
  template <typename T, typename... Args> void switchScene(Args &&...args) {
    scenes_.clear();
    scenes_.push_back(std::make_unique<T>(*this, std::forward<Args>(args)...));
  }

  // add a scene to the stack to create an overlay scene
  template <typename T, typename... Args> void pushScene(Args &&...args) {

    scenes_.push_back(std::make_unique<T>(*this, std::forward<Args>(args)...));
  }

  void popScene();
  Scene *getCurrentScene();
  sf::RenderWindow &getWindow();
  void handleEnvent(const sf::Event &event);
  void update(float dt);
  void render();

private:
  sf::RenderWindow &window_;
  std::vector<std::unique_ptr<Scene>> scenes_;
};
