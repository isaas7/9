#include "../../../../include/application/services/user/service.hpp"

void UserService::addUser(const std::string &username) {
  spdlog::get("console_logger")->debug("UserService::addUser called");

  // Check if the user already exists
  if (userExists(username)) {
    // Decide how to handle the case when the username already exists
    // For example, you can throw an exception or just return without adding
    spdlog::get("console_logger")
        ->warn("User with username '{}' already exists.", username);
    return; // Alternatively, throw an exception or handle it differently
  }

  // If the username doesn't exist, add the user
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
    // Return a default-constructed User if not found
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
