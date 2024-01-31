#pragma once
#include "user.hpp"
#include <string>

class Message {
public:
  Message(const std::string &msg) : message_(msg){};
  std::string getMessage() const { return message_; }

private:
  std::string message_;
};

class PrivateMessage : public Message {
public:
  PrivateMessage(const std::string &msg, const User &sender,
                 const User &receiver)
      : Message(msg), sender_(sender), receiver_(receiver) {}

  User getSender() const { return sender_; }
  User getReceiver() const { return receiver_; }

private:
  User sender_;
  User receiver_;
};
