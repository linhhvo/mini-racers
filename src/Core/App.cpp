#include "App.hpp"
#include "Scenes/MenuScene.hpp"
#include "System/Config.hpp"
#include "System/GameContext.hpp"
#include "System/SceneManager.hpp"
#include "System/AudioManager.hpp"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Vector2.hpp>

App::App(GameContext &context)
    : window(sf::VideoMode(context.config->windowWidth,
                           context.config->windowHeight),
             "MicroMachines Skeleton"),
      context_(context) {
  window.setFramerateLimit(context.config->framerate);
  // Preload all music and sound files
  AudioManager::Instance().LoadMusic("/assets/sounds/music/menu_music.wav");
  AudioManager::Instance().LoadSound("checkpoint_sound", "/assets/sounds/sounds/checkpoint.wav");
}

void App::run() {
  sf::View view = window.getDefaultView();

  SceneManager sceneManager(window);
  sceneManager.switchScene<MenuScene>(context_);

  sf::Clock clock;

  while (window.isOpen()) {
    sf::Event event;

    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        window.close();

      if (event.type == sf::Event::Resized) {
        // match background image size with window size
        view.setSize(event.size.width, event.size.height);
        view.setCenter(event.size.width / 2.f, event.size.height / 2.f);
        window.setView(view);
      }

      sceneManager.handleEnvent(event);
    }

    float dt = clock.restart().asSeconds();

    sceneManager.update(dt);

    // Clear to black by default - scenes can override if needed
    window.clear(sf::Color::Black);

    sceneManager.render();

    window.display();
  }
}
