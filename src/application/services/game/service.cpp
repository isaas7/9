#include "../../../../include/application/services/game/service.hpp"

// service.cpp

GameService::GameService() {}

User GameService::playGame(const User &p1, const User &p2) {
  bool result = game_.flipCoin();

  if (result) {
    // Heads, p1 wins
    spdlog::get("console_logger")->debug("heads, p1 wins");
    return p1;
  } else {
    spdlog::get("console_logger")->debug("tails, p2 wins");
    // Tails, p2 wins
    return p2;
  }
}
