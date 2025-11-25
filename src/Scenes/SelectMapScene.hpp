#pragma once

#include "../System/SceneManager.hpp"
#include "Components/Button/BtnContainer.hpp"
#include "Scene.hpp"
#include "System/GameContext.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>

class SelectMapScene : public Scene {
public:
  SelectMapScene(SceneManager &manager, GameContext &context);
  void handleEvent(const sf::Event &event);
  void update(float dt) {};
  void render();

private:
  sf::Texture backgroundTexture_;
  sf::Sprite backgroundSprite_;
  BtnContainer buttonsContainer_;
  int selectedIndex_ = 0;
  sf::Text confirmationText_;
  bool showConfirmation_ = false;
  sf::Text infoText_;

  SceneManager &manager_;
  GameContext &context_;
};
