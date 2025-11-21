#include "SceneManager.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <memory>

SceneManager::SceneManager(sf ::RenderWindow &window) : window_(window) {};

// remove the overlay scene and go back to the previous scene
void SceneManager::popScene() {
  if (!scenes_.empty()) {
    scenes_.pop_back();
  }
}

Scene *SceneManager::getCurrentScene() {
  if (scenes_.empty()) {
    return nullptr;
  } else {
    return scenes_.back().get();
  }
}

sf::RenderWindow &SceneManager::getWindow() { return window_; }

void SceneManager::handleEnvent(const sf::Event &event) {
  if (!scenes_.empty()) {
    scenes_.back()->handleEvent(event);
  }
}

void SceneManager::update(float dt) {
  if (!scenes_.empty()) {
    scenes_.back()->update(dt);
  }
}

void SceneManager::render() {
  for (auto &scene : scenes_) {
    scene->render();
  }
}
