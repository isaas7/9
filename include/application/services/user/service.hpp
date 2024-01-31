#pragma once
#include "../../../util/util.hpp"
#include "../service.hpp"
#include "user.hpp"

class UserService : public Service {
public:
  void addUser(const std::string &username);
  User getUser(const std::string &username) const;
  bool userExists(const std::string &username) const;

private:
  std::vector<User> users_;
};
