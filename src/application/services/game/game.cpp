#include "../../../../include/application/services/game/game.hpp"

bool Game::flipCoin() {
  int result = rand() % 2;
  return (result == 0);
}
