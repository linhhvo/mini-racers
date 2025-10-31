#pragma once

#include "../System/SceneManager.hpp"
#include "Components/Button/BtnContainer.hpp"
#include "Scene.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

class MenuScene : public Scene {
public:
  MenuScene(SceneManager &manager);
  void handleEvent(const sf::Event &event);
  void update(float dt) {};
  void render();

private:
  sf::Texture backgroundTexture_;
  sf::Sprite backgroundSprite_;
  BtnContainer buttonsContainer_;
  int selectedIndex_ = 0;
  SceneManager &manager_;
  // Button startGameBtn = Button("Start");
};
