// userService.cpp
#include "../../../../include/application/services/user/service.hpp"
#include <string>

void UserService::addUser(const std::string &username) {
  users_.emplace_back(username);
}

User &UserService::getUser(const std::string &username) {
  spdlog::get("console_logger")->debug("UserService::getUser(username)");
  for (auto &user : users_) {
    if (user.getUsername() == username) {
      return user;
    }
  }
  throw std::out_of_range("User not found");
}

bool UserService::userExists(const std::string &username) const {
  for (const auto &user : users_) {
    if (user.getUsername() == username) {
      return true;
    }
  }
  return false;
}

void UserService::updateUsername(User &user, const std::string &username) {
  spdlog::get("console_logger")->debug("UserService::updateUsername");
  getUser(user.getUsername()).updateUsername(username);
}

int UserService::getUsersSize() const { return users_.size(); }

void UserService::incrementGames(User &user) {
  getUser(user.getUsername()).incrementGamesPlayed();
  spdlog::get("console_logger")->debug("gamesPlayed:");
}

int UserService::getGamesPlayed(User &user) {
  return getUser(user.getUsername()).getGamesPlayed();
}
