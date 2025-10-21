#include "App.hpp"
#include "Scenes/MenuScene.hpp"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Vector2.hpp>
#include <Scenes/SceneManager.hpp>

App::App() : window(sf::VideoMode(1280, 960), "MicroMachines Skeleton") {
  window.setFramerateLimit(60);
}

void App::run() {
  sf::View view = window.getDefaultView();

  SceneManager sceneManager(window);
  sceneManager.switchScene<MenuScene>(sceneManager);

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

      if (sceneManager.getCurrentScene()) {
        sceneManager.getCurrentScene()->handleEvent(event);
      }
    }

    float dt = clock.restart().asSeconds();

    if (sceneManager.getCurrentScene()) {
      sceneManager.getCurrentScene()->update(dt);
    }

    window.clear(sf::Color::Black);

    // render menu scene
    if (sceneManager.getCurrentScene()) {
      sceneManager.getCurrentScene()->render();
    }

    window.display();
  }
}
