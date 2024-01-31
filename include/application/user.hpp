#pragma once
#include <string>
class User {
public:
  User(const std::string &username) : username_(username) {}
  std::string getUsername() const { return username_; }

private:
  std::string username_;
};
