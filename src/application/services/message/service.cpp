#include "../../../../include/application/services/message/service.hpp"

void MessageService::addMessage(const std::string &msg) {
  spdlog::get("console_logger")->debug("MessageService::addMessage called");
  messages_.emplace_back(msg);
}

std::vector<Message> MessageService::getMessages() {
  spdlog::get("console_logger")->debug("MessageService::getMessages called");
  return messages_;
}
void PrivateMessageService::addMessage(const User &sender, const User &receiver,
                                       const std::string &msg) {
  spdlog::get("console_logger")
      ->debug("PrivateMessageService::addMessage called");
  privateMessages_.emplace_back(msg, sender, receiver);
}

std::vector<PrivateMessage>
PrivateMessageService::getMessages(const User &sender,
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
