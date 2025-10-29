#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <map>
class InputManager {
public:
  enum MoveInput { UP, DOWN, LEFT, RIGHT, NOTHING };
};

static std::map<sf::Keyboard::Key, InputManager::MoveInput> player1_input{
    {sf::Keyboard::Up, InputManager::UP},
    {sf::Keyboard::Down, InputManager::DOWN},
    {sf::Keyboard::Left, InputManager::LEFT},
    {sf::Keyboard::Right, InputManager::RIGHT}};
