#pragma once
#include "../../../util/util.hpp"
#include "message.hpp"
#include <vector>

class MessageService : public Service {
public:
  void addMessage(const std::string &msg);
  std::vector<Message> getMessages();

private:
  std::vector<Message> messages_;
};
class PrivateMessageService : public Service {
public:
  void addMessage(const User &sender, const User &receiver,
                  const std::string &msg);
  std::vector<PrivateMessage> getMessages(const User &sender,
                                          const User &receiver) const;

private:
  std::vector<PrivateMessage> privateMessages_;
};
