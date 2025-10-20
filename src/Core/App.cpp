#include "App.hpp"

App::App() 
    : window(sf::VideoMode(800, 600), "MicroMachines Skeleton") 
{
    window.setFramerateLimit(60);
}

void App::run() {
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::Black);
        // render stuff here later
        window.display();
    }
}
