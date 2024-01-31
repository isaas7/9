#include "../../../../include/application/services/user/user.hpp"
User::User(const std::string &username) : username_(username) {}

std::string User::getUsername() const { return username_; }
