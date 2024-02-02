#pragma once
#include "../../../../include/util/util.hpp"
#include <string>
class User {
public:
  User(const std::string &username);
  std::string getUsername() const;
  void updateUsername(const std::string &username);
  void incrementGamesPlayed();
  int getGamesPlayed();

private:
  std::string username_;
  int gamesPlayed_;
};
