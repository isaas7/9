#include "../../../../include/application/services/user/service.hpp"

void UserService::addUser(const std::string &username) {
  spdlog::get("console_logger")->debug("UserService::addUser called");
  users_.emplace_back(username);
}

User UserService::getUser(const std::string &username) const {
  auto it =
      std::find_if(users_.begin(), users_.end(), [&username](const User &user) {
        return user.getUsername() == username;
      });

  if (it != users_.end()) {
    return *it;
  } else {
    // Return a default-constructed User if not found (you may want to handle
    // this differently)
    return User("");
  }
}

bool UserService::userExists(const std::string &username) const {
  spdlog::get("console_logger")->debug("UserService::userExists called");
  return std::find_if(users_.begin(), users_.end(),
                      [&username](const User &user) {
                        return user.getUsername() == username;
                      }) != users_.end();
}
