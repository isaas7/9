#pragma once
#include <string>

class User {
public:
  User(const std::string &username);
  std::string getUsername() const;

private:
  std::string username_;
};
