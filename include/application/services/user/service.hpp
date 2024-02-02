#pragma once
#include "../service.hpp"
#include "user.hpp"
#include <string>
#include <vector>

class UserService : public Service {
public:
  void addUser(const std::string &username);
  User &getUser(const std::string &username);
  bool userExists(const std::string &username) const;
  void updateUsername(User &user, const std::string &username);
  int getUsersSize() const;
  void incrementGames(User &user);
  int getGamesPlayed(User &user);

private:
  std::vector<User> users_;
};
