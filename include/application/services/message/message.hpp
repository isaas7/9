#pragma once
#include "../user/service.hpp"
#include <string>

class Message {
public:
  Message(const std::string &msg);
  std::string getMessage() const;

private:
  std::string message_;
};
class PrivateMessage : public Message {
public:
  PrivateMessage(const std::string &msg, const User &sender,
                 const User &receiver);
  User getSender() const;
  User getReceiver() const;

private:
  User sender_;
  User receiver_;
};
