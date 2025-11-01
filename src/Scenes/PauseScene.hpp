#pragma once
#include "../System/SceneManager.hpp"
#include "Scene.hpp"
#include "Scenes/Components/Button/BtnContainer.hpp"
#include "System/GameContext.hpp"
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

class PauseScene : public Scene {
public:
  PauseScene(SceneManager &manager, GameContext &context);
  void handleEvent(const sf::Event &event);
  void update(float dt) {};
  void render();

private:
  GameContext &context_;
  SceneManager &manager_;

  BtnContainer buttonsContainer_;
  int selectedIndex_ = 0;
};
