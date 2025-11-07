#pragma once
#include "../System/SceneManager.hpp"
#include "Scene.hpp"
#include "System/GameContext.hpp"
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>

class VictoryScene : public Scene {
public:
  VictoryScene(SceneManager &manager, GameContext &context, int winner);
  void handleEvent(const sf::Event &event);
  void update(float dt) {};
  void render();

private:
  GameContext &context_;
  SceneManager &manager_;
  sf::RectangleShape background_;
  sf::Texture bannerTexture_;
  sf::Sprite bannerSprite_;
  sf::Text infoText_;
  sf::Text raceResult_;
  int winner_;
};
