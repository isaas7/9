#pragma once
#include "../util/util.hpp"
#include "message.hpp"
#include "user.hpp"
#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
class Service {
public:
  virtual ~Service() = default;
};

class MessageService : public Service {
public:
  void addMessage(const std::string &msg) {
    spdlog::get("console_logger")->debug("MessageService::addMessage called");
    messages_.emplace_back(msg);
  }

  std::vector<Message> getMessages() {
    spdlog::get("console_logger")->debug("MessageService::getMessages called");
    return messages_;
  }

private:
  std::vector<Message> messages_;
};

class UserService : public Service {
public:
  void addUser(const std::string &username) {
    spdlog::get("console_logger")->debug("UserService::addUser called");
    users_.emplace_back(username);
  }
  User getUser(const std::string &username) const {
    auto it = std::find_if(users_.begin(), users_.end(),
                           [&username](const User &user) {
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

  bool userExists(const std::string &username) const {
    spdlog::get("console_logger")->debug("UserService::userExists called");
    return std::find_if(users_.begin(), users_.end(),
                        [&username](const User &user) {
                          return user.getUsername() == username;
                        }) != users_.end();
  }

private:
  std::vector<User> users_;
};

class PrivateMessageService : public Service {
public:
  void addMessage(const User &sender, const User &receiver,
                  const std::string &msg) {
    spdlog::get("console_logger")
        ->debug("PrivateMessageService::addMessage called");
    privateMessages_.emplace_back(msg, sender, receiver);
  }

  std::vector<PrivateMessage> getMessages(const User &sender,
                                          const User &receiver) const {
    spdlog::get("console_logger")
        ->debug("PrivateMessageService::getMessages called");
    std::vector<PrivateMessage> result;
    for (const auto &message : privateMessages_) {
      if (message.getSender().getUsername() == sender.getUsername() &&
          message.getReceiver().getUsername() == receiver.getUsername()) {
        result.emplace_back(message);
      }
    }
    return result;
  }

private:
  std::vector<PrivateMessage> privateMessages_;
};
class Application {
public:
  Application()
      : messageService_(std::make_shared<MessageService>()),
        userService_(std::make_shared<UserService>()),
        privateMessageService_(std::make_shared<PrivateMessageService>()) {}

  virtual ~Application() = default;

  std::shared_ptr<MessageService> getMessageService() const {
    spdlog::get("console_logger")
        ->debug("Application::getMessageService called");
    return messageService_;
  }

  std::shared_ptr<PrivateMessageService> getPrivateMessageService() const {
    spdlog::get("console_logger")
        ->debug("Application::getPrivateMessageService called");
    return privateMessageService_;
  }

  std::shared_ptr<UserService> getUserService() const {
    spdlog::get("console_logger")->debug("Application::getUserService called");
    return userService_;
  }

private:
  std::shared_ptr<MessageService> messageService_;
  std::shared_ptr<PrivateMessageService> privateMessageService_;
  std::shared_ptr<UserService> userService_;
};
