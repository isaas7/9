#include "../../../../include/application/services/user/user.hpp"

User::User(const std::string &username) : username_(username), gamesPlayed_(0) {
  spdlog::get("console_logger")->debug("User::User");
}

std::string User::getUsername() const {
  spdlog::get("console_logger")->debug("User::getUsername()");
  return username_;
}

void User::updateUsername(const std::string &username) {
  spdlog::get("console_logger")
      ->debug("User::updateUsername, old username_: {}", username_);
  username_ = username;
  spdlog::get("console_logger")->debug("(new) username_: {}", username_);
}

void User::incrementGamesPlayed() { gamesPlayed_++; }

int User::getGamesPlayed() { return gamesPlayed_; }
