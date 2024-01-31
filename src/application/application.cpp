#include "../../include/application/application.hpp"

Application::Application()
    : messageService_(std::make_shared<MessageService>()),
      userService_(std::make_shared<UserService>()),
      privateMessageService_(std::make_shared<PrivateMessageService>()) {}

Application::~Application() = default;

std::shared_ptr<MessageService> Application::getMessageService() const {
  spdlog::get("console_logger")->debug("Application::getMessageService called");
  return messageService_;
}

std::shared_ptr<PrivateMessageService>
Application::getPrivateMessageService() const {
  spdlog::get("console_logger")
      ->debug("Application::getPrivateMessageService called");
  return privateMessageService_;
}

std::shared_ptr<UserService> Application::getUserService() const {
  spdlog::get("console_logger")->debug("Application::getUserService called");
  return userService_;
}
