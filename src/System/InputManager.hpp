#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <map>
#include <optional>
#include <vector>
class InputManager {
public:
  enum InputAction { UP, DOWN, LEFT, RIGHT, RELEASED };

  std::optional<int> getPlayerFromKey(sf::Keyboard::Key key) {
    for (unsigned int i = 0; i < PlayerInput.size(); i++) {
      if (PlayerInput[i].contains(key)) {
        return i;
      }
    }
    return std::nullopt;
  }

  InputAction getPlayerAction(sf::Keyboard::Key key, int playerIdx) {
    return PlayerInput[playerIdx].at(key);
  }

private:
  std::map<sf::Keyboard::Key, InputManager::InputAction> leftPlayer_input{
      {sf::Keyboard::W, InputManager::UP},
      {sf::Keyboard::S, InputManager::DOWN},
      {sf::Keyboard::A, InputManager::LEFT},
      {sf::Keyboard::D, InputManager::RIGHT}};

  std::map<sf::Keyboard::Key, InputManager::InputAction> rightPlayer_input{
      {sf::Keyboard::Up, InputManager::UP},
      {sf::Keyboard::Down, InputManager::DOWN},
      {sf::Keyboard::Left, InputManager::LEFT},
      {sf::Keyboard::Right, InputManager::RIGHT}};

  std::vector<std::map<sf::Keyboard::Key, InputManager::InputAction>>
      PlayerInput{leftPlayer_input, rightPlayer_input};
};
